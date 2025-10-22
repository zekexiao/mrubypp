//
// Created by ZekeXiao on 2025/10/17.
//

#ifndef MRUBYPP_MRUBYPP_UTILS_H
#define MRUBYPP_MRUBYPP_UTILS_H

#include <mruby.h>

namespace mrubypp {
// gc arena
class arena_guard {
public:
  explicit arena_guard(mrb_state *mrb) : mrb(mrb) {
    ai = mrb_gc_arena_save(mrb);
  }
  arena_guard(arena_guard &&other) = delete;
  arena_guard(const arena_guard &other) = delete;
  ~arena_guard() { mrb_gc_arena_restore(mrb, ai); }

  // arena_idx
  [[nodiscard]] int get_ai() const { return ai; }

private:
  mrb_state *mrb;
  int ai;
};
} // namespace mrubypp
#endif // MRUBYPP_MRUBYPP_UTILS_H
