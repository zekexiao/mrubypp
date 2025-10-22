#ifndef MRUBYPP_STD_CONVERTERS_H
#define MRUBYPP_STD_CONVERTERS_H

#include "arena_guard.h"

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <string>
#include <vector>

namespace mrubypp {

template <typename T> struct converter {
  static mrb_value to_mrb(mrb_state *mrb, const T &var) {
    static_assert(sizeof(T) == 0, "Specialization required");
    return mrb_nil_value();
  }
  static T from_mrb(mrb_state *mrb, mrb_value value) {
    static_assert(sizeof(T) == 0, "Specialization required");
    return T();
  }
};

template <> struct converter<int> {
  static mrb_value to_mrb(mrb_state *mrb, int var) {
    return mrb_fixnum_value(var);
  }
  static int from_mrb(mrb_state *mrb, mrb_value value) {
    return mrb_fixnum(value);
  }
};

template <> struct converter<unsigned int> {
  static mrb_value to_mrb(mrb_state *mrb, unsigned int var) {
    return mrb_fixnum_value(var);
  }
  static int from_mrb(mrb_state *mrb, mrb_value value) {
    return mrb_fixnum(value);
  }
};

template <> struct converter<float> {
  static mrb_value to_mrb(mrb_state *mrb, float var) {
    return mrb_float_value(mrb, var);
  }
  static double from_mrb(mrb_state *mrb, mrb_value value) {
    return mrb_float(value);
  }
};

template <> struct converter<double> {
  static mrb_value to_mrb(mrb_state *mrb, double var) {
    return mrb_float_value(mrb, var);
  }
  static double from_mrb(mrb_state *mrb, mrb_value value) {
    return mrb_float(value);
  }
};

template <> struct converter<std::string> {
  static mrb_value to_mrb(mrb_state *mrb, const std::string &var) {
    return mrb_str_new(mrb, var.data(), (mrb_int)var.size());
  }

  static std::string from_mrb(mrb_state *mrb, mrb_value value) {
    if (mrb_string_p(value)) {
      return std::string(RSTRING_PTR(value), RSTRING_LEN(value));
    }
    return {};
  }
};

template <> struct converter<const char *> {
  static mrb_value to_mrb(mrb_state *mrb, const char *var) {
    return mrb_str_new(mrb, var, (mrb_int)strlen(var));
  }
};

template <typename T> struct converter<std::vector<T>> {
  static mrb_value to_mrb(mrb_state *mrb, const std::vector<T> &var) {
    mrb_value ary = mrb_ary_new_capa(mrb, static_cast<mrb_int>(var.size()));
    for (const T &el : var) {
      arena_guard guard(mrb);
      mrb_ary_push(mrb, ary, converter<T>::to_mrb(mrb, el));
    }
    return ary;
  }

  static std::vector<T> from_mrb(mrb_state *mrb, mrb_value value) {
    arena_guard guard(mrb);
    std::vector<T> result;
    if (mrb_array_p(value)) {
      int len = RARRAY_LEN(value);
      for (int i = 0; i < len; ++i) {
        result.append(converter<T>::from_mrb(mrb, mrb_ary_ref(mrb, value, i)));
      }
    }
    return result;
  }
};

} // namespace mrubypp

#endif // MRUBYPP_STD_CONVERTERS_H
