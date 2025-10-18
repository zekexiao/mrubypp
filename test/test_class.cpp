//
// Created by ZekeXiao on 2025/10/18.
//

#include <catch2/catch_all.hpp>

#include "mrubypp.h"

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
    mrb_value obj = mrb_obj_value(
        mrb_data_object_alloc(mrb, mrb->object_class, new Point(var),
                              &mrubypp_class_builder<Point>::data_type));
    return obj;
  }

  static Point from_mrb(mrb_state *mrb, mrb_value value) {
    if (mrb_type(value) == MRB_TT_DATA) {
      Point *point = static_cast<Point *>(DATA_PTR(value));
      return *point;
    }
    return Point(0, 0);
  }
};

TEST_CASE("Point", "[class]") {
  mrubypp engine;
  engine.class_builder<Point>("Point")
      .def_constructor<int, int>()
      .def_method("add", &Point::add)
      .def_class_method("none", &Point::none)
      .def_property("x", &Point::get_x, &Point::set_x);

  engine.load(R"(
    def test_point()
      p = Point.new(3, 4)
      p.x = 10
      p.add(p)
      p.x += Point::none()
      return p
    end
  )");

  auto point_result = engine.call<Point>("test_point");

  REQUIRE(point_result.get_x() == 21);
  REQUIRE(point_result.get_y() == 8);
}