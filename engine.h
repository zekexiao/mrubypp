#ifndef MRUBYPP_H
#define MRUBYPP_H

#include "arena_guard.h"
#include "bind_class.h"
#include "converters.h"

#include <functional>
#include <mruby.h>
#include <string>

#include <mruby/compile.h>

namespace mrubypp {
class engine {
public:
  engine() { mrb_ = mrb_open(); }
  ~engine() { mrb_close(mrb_); }
  void load(const std::string &str) {
    if (!mrb_)
      return;
    mrb_load_string(mrb_, str.c_str());
  }

  template <typename T> T call(const std::string &funcName) {
    arena_guard guard(mrb_);
    mrb_value result =
        mrb_funcall(mrb_, mrb_top_self(mrb_), funcName.data(), 0);
    return converter<T>::from_mrb(mrb_, result);
  }

  template <typename T, typename... Args>
  T call(const std::string &funcName, Args... args) {
    arena_guard guard(mrb_);
    mrb_value argv[] = {converter<Args>::to_mrb(mrb_, args)...};
    mrb_sym sym = mrb_intern_cstr(mrb_, funcName.data());
    mrb_value result =
        mrb_funcall_argv(mrb_, mrb_top_self(mrb_), sym, sizeof...(Args), argv);
    return converter<T>::from_mrb(mrb_, result);
  }

  template <typename T>
  class_builder<T> class_builder(const std::string &class_name) {
    return class_builder<T>(mrb_, class_name);
  }

  [[nodiscard]] mrb_state *get_mrb() const { return mrb_; }

private:
  mrb_state *mrb_;
};
} // namespace mrubypp

#endif // MRUBYPP_H
