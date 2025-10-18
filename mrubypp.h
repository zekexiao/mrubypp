#ifndef MRUBYPP_H
#define MRUBYPP_H

#include "mrubypp_arena_guard.h"
#include "mrubypp_bind_class.h"
#include "mrubypp_converters.h"

#include <functional>
#include <mruby.h>
#include <string>

#include <mruby/compile.h>

// 支持自定义类型注册
class mrubypp {
public:
  mrubypp() { mrb = mrb_open(); }
  ~mrubypp() { mrb_close(mrb); }
  void load(const std::string &str) {
    if (!mrb)
      return;
    mrb_load_string(mrb, str.c_str());
  }

  template <typename T> T call(const std::string &funcName) {
    mrubypp_arena_guard guard(mrb);
    mrb_value result = mrb_funcall(mrb, mrb_top_self(mrb), funcName.data(), 0);
    return mrubypp_converter<T>::from_mrb(mrb, result);
  }

  template <typename T, typename... Args>
  T call(const std::string &funcName, Args... args) {
    mrubypp_arena_guard guard(mrb);
    mrb_value argv[] = {mrubypp_converter<Args>::to_mrb(mrb, args)...};
    mrb_sym sym = mrb_intern_cstr(mrb, funcName.data());
    mrb_value result =
        mrb_funcall_argv(mrb, mrb_top_self(mrb), sym, sizeof...(Args), argv);
    return mrubypp_converter<T>::from_mrb(mrb, result);
  }

  template <typename T>
  mrubypp_class_builder<T> class_builder(const std::string &class_name) {
    return mrubypp_class_builder<T>(mrb, class_name);
  }

  [[nodiscard]] mrb_state *get_mrb() const { return mrb; }

private:
  mrb_state *mrb;
};

#endif // MRUBYPP_H
