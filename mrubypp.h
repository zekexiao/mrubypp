#ifndef MRUBYPP_H
#define MRUBYPP_H

#include "mrubypp_arena_guard.h"
#include "mrubypp_bind_class.h"
#include "mrubypp_converters.h"

#include <functional>
#include <mruby.h>
#include <string>

#include <mruby/compile.h>

class mrubypp {
public:
  mrubypp() { mrb_ = mrb_open(); }
  ~mrubypp() { mrb_close(mrb_); }
  void load(const std::string &str) {
    if (!mrb_)
      return;
    mrb_load_string(mrb_, str.c_str());
  }

  template <typename T> T call(const std::string &funcName) {
    mrubypp_arena_guard guard(mrb_);
    mrb_value result =
        mrb_funcall(mrb_, mrb_top_self(mrb_), funcName.data(), 0);
    return mrubypp_converter<T>::from_mrb(mrb_, result);
  }

  template <typename T, typename... Args>
  T call(const std::string &funcName, Args... args) {
    mrubypp_arena_guard guard(mrb_);
    mrb_value argv[] = {mrubypp_converter<Args>::to_mrb(mrb_, args)...};
    mrb_sym sym = mrb_intern_cstr(mrb_, funcName.data());
    mrb_value result =
        mrb_funcall_argv(mrb_, mrb_top_self(mrb_), sym, sizeof...(Args), argv);
    return mrubypp_converter<T>::from_mrb(mrb_, result);
  }

  template <typename T>
  mrubypp_class_builder<T> class_builder(const std::string &class_name) {
    return mrubypp_class_builder<T>(mrb_, class_name);
  }

  [[nodiscard]] mrb_state *get_mrb() const { return mrb_; }

private:
  mrb_state *mrb_;
};

#endif // MRUBYPP_H
