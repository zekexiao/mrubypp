//
// Created by ZekeXiao on 2025/10/17.
//

#ifndef MRUBYPP_MRUBYPP_UTILS_H
#define MRUBYPP_MRUBYPP_UTILS_H

#include <mruby.h>

// gc arena
class mrubypp_arena_guard {
public:
  explicit mrubypp_arena_guard(mrb_state *mrb) : mrb(mrb) {
    ai = mrb_gc_arena_save(mrb);
  }
  mrubypp_arena_guard(mrubypp_arena_guard &&other) = delete;
  mrubypp_arena_guard(const mrubypp_arena_guard &other) = delete;
  ~mrubypp_arena_guard() { mrb_gc_arena_restore(mrb, ai); }

  // arena_idx
  [[nodiscard]] int get_ai() const { return ai; }

private:
  mrb_state *mrb;
  int ai;
};
#endif // MRUBYPP_MRUBYPP_UTILS_H
