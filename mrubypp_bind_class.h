#ifndef MRUBYPP_BIND_CLASS
#define MRUBYPP_BIND_CLASS

#include "mrubypp_arena_guard.h"
#include "mrubypp_converters.h"

#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <mruby/class.h>
#include <mruby/data.h>

template <typename... Args, size_t... Is>
std::tuple<Args...> mrubypp_get_args_helper_impl(mrb_state *mrb,
                                                 const mrb_value *argv,
                                                 std::index_sequence<Is...>) {
  return std::make_tuple(
      mrubypp_converter<typename std::decay<Args>::type>::from_mrb(
          mrb, argv[Is])...);
}

template <typename... Args>
std::tuple<typename std::decay<Args>::type...>
mrubypp_get_args_helper(mrb_state *mrb) {
  mrubypp_arena_guard guard(mrb);
  auto argc = mrb_get_argc(mrb);
  auto argv = mrb_get_argv(mrb);

  if (argc != sizeof...(Args)) {
    mrb_raise(mrb, E_ARGUMENT_ERROR, "argument count mismatch");
  }

  return mrubypp_get_args_helper_impl<typename std::decay<Args>::type...>(
      mrb, argv, std::index_sequence_for<Args...>{});
}

template <typename T, typename Ret, typename... MethodArgs>
struct mrubypp_method_wrapper {
  static Ret (T::*method)(MethodArgs...);

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = mrubypp_get_args_helper<MethodArgs...>(mrb);
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid instance");
    }

    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrb_nil_value();
    } else {
      Ret result =
          std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrubypp_converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename T, typename Ret, typename... MethodArgs>
struct mrubypp_const_method_wrapper {
  static Ret (T::*method)(MethodArgs...) const;

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = mrubypp_get_args_helper<MethodArgs...>(mrb);
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid instance");
    }

    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrb_nil_value();
    } else {
      Ret result =
          std::apply(method, std::tuple_cat(std::make_tuple(instance), args));
      return mrubypp_converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename Ret, typename... FuncArgs> struct mrubypp_function_wrapper {
  static Ret (*func)(FuncArgs...);

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = mrubypp_get_args_helper<FuncArgs...>(mrb);
    if constexpr (std::is_same_v<Ret, void>) {
      std::apply(func, args);
      return mrb_nil_value();
    } else {
      Ret result = std::apply(func, args);
      return mrubypp_converter<Ret>::to_mrb(mrb, result);
    }
  }
};

template <typename T, typename Ret> struct mrubypp_getter_wrapper {
  static Ret (T::*getter)() const;

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid instance");
    }

    Ret result = (instance->*getter)();
    return mrubypp_converter<Ret>::to_mrb(mrb, result);
  }
};

template <typename T, typename Ret> struct mrubypp_setter_wrapper {
  static void (T::*setter)(Ret);

  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = mrubypp_get_args_helper<Ret>(mrb);
    T *instance = static_cast<T *>(DATA_PTR(self));
    if (!instance) {
      mrb_raise(mrb, E_RUNTIME_ERROR, "Invalid instance");
    }

    std::apply(setter, std::tuple_cat(std::make_tuple(instance), args));
    return self;
  }
};
template <typename T> class mrubypp_class_builder;

template <typename T, typename... Args> struct mrubypp_constructor_wrapper {
  static mrb_value wrapper(mrb_state *mrb, mrb_value self) {
    auto args = mrubypp_get_args_helper<Args...>(mrb);
    T *instance = std::apply([](Args... args) { return new T(args...); }, args);

    DATA_PTR(self) = instance;
    DATA_TYPE(self) = &mrubypp_class_builder<T>::data_type;
    return self;
  }
};

template <typename T> class mrubypp_class_builder {
private:
  mrb_state *mrb_;
  struct RClass *rclass_;
  std::string name_;

  static void free_instance(mrb_state *mrb, void *ptr) {
    if (ptr) {
      T *instance = static_cast<T *>(ptr);
      delete instance;
    }
  }

public:
  static const struct mrb_data_type data_type;

  mrubypp_class_builder(mrb_state *mrb, std::string name)
      : mrb_(mrb), name_(std::move(name)) {
    rclass_ = mrb_define_class(mrb_, name_.c_str(), mrb_->object_class);
    MRB_SET_INSTANCE_TT(rclass_, MRB_TT_DATA);

    mrb_define_method(
        mrb_, rclass_, "initialize",
        [](mrb_state *mrb, mrb_value self) -> mrb_value {
          mrb_raise(mrb, E_NOTIMP_ERROR,
                    "Initialize must be defined explicitly");
          return self;
        },
        MRB_ARGS_ANY());
  }

  template <typename... Args> mrubypp_class_builder &def_constructor() {
    mrb_define_method(mrb_, rclass_, "initialize",
                      &mrubypp_constructor_wrapper<T, Args...>::wrapper,
                      MRB_ARGS_REQ(sizeof...(Args)));

    return *this;
  }

  template <typename Ret, typename... MethodArgs>
  mrubypp_class_builder &def_method(const std::string &name,
                                    Ret (T::*method)(MethodArgs...)) {
    mrubypp_method_wrapper<T, Ret, MethodArgs...>::method = method;

    mrb_define_method(mrb_, rclass_, name.c_str(),
                      &mrubypp_method_wrapper<T, Ret, MethodArgs...>::wrapper,
                      MRB_ARGS_REQ(sizeof...(MethodArgs)));

    return *this;
  }

  template <typename Ret, typename... MethodArgs>
  mrubypp_class_builder &def_method(const std::string &name,
                                    Ret (T::*method)(MethodArgs...) const) {
    mrubypp_const_method_wrapper<T, Ret, MethodArgs...>::method = method;

    mrb_define_method(
        mrb_, rclass_, name.c_str(),
        &mrubypp_const_method_wrapper<T, Ret, MethodArgs...>::wrapper,
        MRB_ARGS_REQ(sizeof...(MethodArgs)));

    return *this;
  }

  template <typename Ret, typename... FuncArgs>
  mrubypp_class_builder &def_class_method(const std::string &name,
                                          Ret (*func)(FuncArgs...)) {
    mrubypp_function_wrapper<Ret, FuncArgs...>::func = func;

    mrb_define_class_method(
        mrb_, rclass_, name.c_str(),
        &mrubypp_function_wrapper<Ret, FuncArgs...>::wrapper,
        MRB_ARGS_REQ(sizeof...(FuncArgs)));

    return *this;
  }

  template <typename Ret, typename... MethodArgs>
  mrubypp_class_builder &def_property(const std::string &name,
                                      Ret (T::*getter)() const,
                                      void (T::*setter)(Ret)) {
    mrubypp_getter_wrapper<T, Ret>::getter = getter;
    mrb_define_method(mrb_, rclass_, name.c_str(),
                      &mrubypp_getter_wrapper<T, Ret>::wrapper,
                      MRB_ARGS_NONE());

    std::string setter_name = name + "=";
    mrubypp_setter_wrapper<T, Ret>::setter = setter;
    mrb_define_method(mrb_, rclass_, setter_name.c_str(),
                      &mrubypp_setter_wrapper<T, Ret>::wrapper,
                      MRB_ARGS_REQ(1));

    return *this;
  }

  [[nodiscard]] struct RClass *get_rclass() const { return rclass_; }
};

template <typename T>
const struct mrb_data_type mrubypp_class_builder<T>::data_type = {
  typeid(T).name(), mrubypp_class_builder<T>::free_instance
};

template <typename T, typename Ret, typename... MethodArgs>
Ret (T::*mrubypp_method_wrapper<T, Ret, MethodArgs...>::method)(MethodArgs...);

template <typename T, typename Ret, typename... MethodArgs>
Ret (T::*mrubypp_const_method_wrapper<T, Ret, MethodArgs...>::method)(
    MethodArgs...) const;

template <typename Ret, typename... FuncArgs>
Ret (*mrubypp_function_wrapper<Ret, FuncArgs...>::func)(FuncArgs...);

template <typename T, typename Ret>
Ret (T::*mrubypp_getter_wrapper<T, Ret>::getter)() const;

template <typename T, typename Ret>
void (T::*mrubypp_setter_wrapper<T, Ret>::setter)(Ret);

#endif
