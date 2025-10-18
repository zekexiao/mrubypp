//
// Created by ZekeXiao on 2025/10/18.
//

#ifndef MRUBYPP_POINT_H
#define MRUBYPP_POINT_H

#include "mrubypp_converters.h"

#include "mruby/data.h"

#include "mrubypp_bind_class.h"

class Point {
public:
  Point(int x, int y) : x_(x), y_(y) {}

  void set_values(int x, int y) {
    x_ = x;
    y_ = y;
  }

  int get_x() const { return x_; }
  void set_x(int x) { x_ = x; }

  int get_y() const { return y_; }
  void set_y(int y) { y_ = y; }

  void add(const Point &other) {
    this->x_ += other.x_;
    this->y_ += other.y_;
  }
  static int none() { return 1; }

private:
  int x_;
  int y_;
};

template <> struct mrubypp_converter<Point> {
  static mrb_value to_mrb(mrb_state *mrb, const Point &var) {
    // 创建一个新的 Point 对象数据结构
    mrb_value obj = mrb_obj_value(
        mrb_data_object_alloc(mrb, mrb->object_class, new Point(var),
                              &mrubypp_class_builder<Point>::data_type));
    return obj;
  }

  static Point from_mrb(mrb_state *mrb, mrb_value value) {
    // 从 mrb_value 中提取 Point 对象
    if (mrb_type(value) == MRB_TT_DATA) {
      Point *point = static_cast<Point *>(DATA_PTR(value));
      return *point;
    }
    return Point(0, 0); // 默认构造
  }
};
#endif // MRUBYPP_POINT_H
