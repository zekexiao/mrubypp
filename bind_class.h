#ifndef MRUBYPP_BIND_CLASS
#define MRUBYPP_BIND_CLASS

#include "arena_guard.h"
#include "converters.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <mruby/class.h>
#include <mruby/data.h>
#include <mruby/proc.h>

namespace mrubypp {
template <typename... Args, size_t... Is>
std::tuple<Args...> get_args_helper_impl(mrb_state *mrb, const mrb_value *argv,
                                         std::index_sequence<Is...>) {
  return std::make_tuple(
      converter<typename std::decay<Args>::type>::from_mrb(mrb, argv[Is])...);
}

template <typename... Args>
std::tuple<typename std::decay<Args>::type...> get_args_helper(mrb_state *mrb) {
  arena_guard guard(mrb);
  auto argc = mrb_get_argc(mrb);
  auto argv = mrb_get_argv(mrb);

  if (argc != sizeof...(Args)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "argument count mismatch");
  }

  return get_args_helper_impl<typename std::decay<Args>::type...>(
      mrb, argv, std::index_sequence_for<Args...>{});
}

template <typename T, typename Ret, typename... MethodArgs>
struct method_wrapper {
  using MethodType = Ret (T::*)(MethodArgs...);

  // TODO, fix memory leak
  static void *method_to_ptr(MethodType method) {
    auto ptr = malloc(sizeof(MethodType));
    memcpy(ptr, &method, sizeof(MethodType));
    return ptr;
  }

  static MethodType ptr_to_method(void *ptr) {
    MethodType method;
    memcpy(&method, ptr, sizeof(MethodType));
    return method;
  }

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = get_args_helper<MethodArgs...>(mrb);
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "invalid instance");
    }

    void *method_ptr = mrb_cptr(mrb_cfunc_env_get(mrb, 0));
    auto method = ptr_to_method(method_ptr);

    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrb_nil_value();
    } else {
      Ret result =
          std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename T, typename Ret, typename... MethodArgs>
struct const_method_wrapper {
  using MethodType = Ret (T::*)(MethodArgs...) const;

  // TODO, fix memory leak
  static void *method_to_ptr(MethodType method) {
    auto ptr = malloc(sizeof(MethodType));
    memcpy(ptr, &method, sizeof(MethodType));
    return ptr;
  }

  static MethodType ptr_to_method(void *ptr) {
    MethodType method;
    memcpy(&method, ptr, sizeof(MethodType));
    return method;
  }

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = get_args_helper<MethodArgs...>(mrb);
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "invalid instance");
    }

    void *method_ptr = mrb_cptr(mrb_cfunc_env_get(mrb, 0));
    auto method = ptr_to_method(method_ptr);
    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrb_nil_value();
    } else {
      Ret result =
          std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename T, typename Ret, typename... FuncArgs>
struct function_wrapper {
  using FunctionType = Ret (*)(FuncArgs...);

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto func = (FunctionType)mrb_cptr(mrb_cfunc_env_get(mrb, 0));
    auto args = get_args_helper<FuncArgs...>(mrb);
    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(func, args);
      return mrb_nil_value();
    } else {
      Ret result = std::apply(func, args);
      return converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename T> class class_builder;

template <typename T, typename... Args> struct constructor_wrapper {
  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = get_args_helper<Args...>(mrb);
    T *instance = std::apply([](Args... args) { return new T(args...); }, args);

    DATA_PTR(self) = instance;
    DATA_TYPE(self) = &class_builder<T>::data_type;
    return self;
  }
};

template <typename T> class class_builder {
public:
  static const struct mrb_data_type data_type;

  class_builder(mrb_state *mrb, std::string name)
      : mrb_(mrb), name_(std::move(name)) {
    rclass_ = mrb_define_class(mrb_, name_.c_str(), mrb_->object_class);
    MRB_SET_INSTANCE_TT(rclass_, MRB_TT_DATA);

    mrb_define_method(
        mrb_, rclass_, "initialize",
        [](mrb_state *mrb, mrb_value self) -> mrb_value {
          mrb_raise(mrb, E_NOTIMP_ERROR,
                    "initialize must be defined explicitly");
          return self;
        },
        MRB_ARGS_ANY());
  }

  template <typename... Args> class_builder &def_constructor() {
    mrb_define_method(mrb_, rclass_, "initialize",
                      &constructor_wrapper<T, Args...>::wrapper,
                      MRB_ARGS_REQ(sizeof...(Args)));
    return *this;
  }

  template <typename Ret, typename... MethodArgs>
  class_builder &def_method(const std::string &name,
                            Ret (T::*method)(MethodArgs...)) {
    mrb_sym method_sym = mrb_intern_cstr(mrb_, name.c_str());

    using wrap = method_wrapper<T, Ret, MethodArgs...>;
    auto ptr = wrap::method_to_ptr(method);

    mrb_value env[] = {
        mrb_cptr_value(mrb_, ptr),
    };
    struct RProc *p = mrb_proc_new_cfunc_with_env(mrb_, &wrap::wrapper, 1, env);
    mrb_method_t m;
    MRB_METHOD_FROM_PROC(m, p);
    mrb_define_method_raw(mrb_, rclass_, method_sym, m);
    //  MRB_ARGS_REQ(sizeof...(MethodArgs))
    return *this;
  }

  template <typename Ret, typename... MethodArgs>
  class_builder &def_method(const std::string &name,
                            Ret (T::*method)(MethodArgs...) const) {
    mrb_sym method_sym = mrb_intern_cstr(mrb_, name.c_str());
    using wrap = const_method_wrapper<T, Ret, MethodArgs...>;

    auto ptr = wrap::method_to_ptr(method);
    auto obj = mrb_cptr_value(mrb_, ptr);
    mrb_gc_protect(mrb_, obj);
    mrb_value env[] = {obj};

    struct RProc *p = mrb_proc_new_cfunc_with_env(mrb_, &wrap::wrapper, 1, env);
    mrb_method_t m;
    MRB_METHOD_FROM_PROC(m, p);
    mrb_define_method_raw(mrb_, rclass_, method_sym, m);
    return *this;
  }

  template <typename Ret, typename... FuncArgs>
  class_builder &def_class_method(const std::string &name,
                                  Ret (*func)(FuncArgs...)) {
    using wrap = function_wrapper<T, Ret, FuncArgs...>;
    mrb_value env[] = {
        mrb_cptr_value(mrb_, (void *)func),
    };

    mrb_sym func_sym = mrb_intern_cstr(mrb_, name.c_str());
    struct RProc *p = mrb_proc_new_cfunc_with_env(mrb_, &wrap::wrapper, 1, env);
    mrb_method_t m;
    MRB_METHOD_FROM_PROC(m, p);

    // register and replace
    mrb_define_class_method_id(mrb_, rclass_, func_sym, NULL, MRB_ARGS_ANY());
    mrb_define_method_raw(mrb_, rclass_->c, func_sym, m);
    return *this;
  }

  // get set
  template <typename Ret>
  class_builder &def_property(const std::string &name, Ret (T::*getter)() const,
                              void (T::*setter)(Ret)) {
    def_method(name, getter);
    def_method(name + "=", setter);
    return *this;
  }

  class_builder &def_native(const std::string &name, mrb_func_t func,
                            mrb_aspec aspec = MRB_ARGS_ANY()) {
    mrb_define_method(mrb_, rclass_, name.c_str(), func, aspec);
    return *this;
  }

  [[nodiscard]] struct RClass *get_rclass() const { return rclass_; }

  [[nodiscard]] static T *get_this(mrb_state *mrb, mrb_value value) {
    T *instance = static_cast<T *>(DATA_PTR(value));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "invalid instance");
    }
    return instance;
  }

private:
  static void free_instance(mrb_state *mrb, void *ptr) {
    if (ptr) {
      T *instance = static_cast<T *>(ptr);
      delete instance;
    }
  }

  mrb_state *mrb_;
  struct RClass *rclass_;
  std::string name_;
};

template <typename T> const struct mrb_data_type class_builder<T>::data_type = {
  typeid(T).name(), class_builder<T>::free_instance
};
} // namespace mrubypp

#endif
