#ifndef MRUBYPP_H
#define MRUBYPP_H

#include "arena_guard.h"
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

  template <typename T, typename... Args>
  typename std::enable_if<!std::is_same<T, void>::value, T>::type
  call(const std::string &funcName, Args... args) {
    arena_guard guard(mrb_);
    if constexpr (sizeof...(Args) > 0) {
      mrb_value argv[] = {converter<Args>::to_mrb(mrb_, args)...};
      mrb_sym sym = mrb_intern_cstr(mrb_, funcName.data());
      mrb_value result = mrb_funcall_argv(mrb_, mrb_top_self(mrb_), sym,
                                          sizeof...(Args), argv);
      return converter<T>::from_mrb(mrb_, result);
    }
    mrb_value result =
        mrb_funcall(mrb_, mrb_top_self(mrb_), funcName.data(), 0);
    return converter<T>::from_mrb(mrb_, result);
  }

  template <typename T = void, typename... Args>
  typename std::enable_if<std::is_same<T, void>::value, T>::type
  call(const std::string &funcName, Args... args) {
    if constexpr (sizeof...(Args) > 0) {
      arena_guard guard(mrb_);
      mrb_value argv[] = {converter<Args>::to_mrb(mrb_, args)...};
      mrb_sym sym = mrb_intern_cstr(mrb_, funcName.data());
      mrb_funcall_argv(mrb_, mrb_top_self(mrb_), sym, sizeof...(Args), argv);
    } else {
      mrb_funcall(mrb_, mrb_top_self(mrb_), funcName.data(), 0);
    }
  }

  [[nodiscard]] mrb_state *get_mrb() const { return mrb_; }

private:
  mrb_state *mrb_;
};
} // namespace mrubypp

#endif // MRUBYPP_H
