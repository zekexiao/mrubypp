//
// Created by ZekeXiao on 2025/10/18.
//

#include <catch2/catch_all.hpp>

#include <mrubypp/bind_class.h>
#include <mrubypp/engine.h>

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

static mrb_value point_native_div(mrb_state *mrb, mrb_value self) {
  auto point = mrubypp::bind_class<Point>::get_this(mrb, self);
  auto divisor = mrubypp::converter<int>::from_mrb(mrb, mrb_get_arg1(mrb));
  point->set_x(point->get_x() / divisor);
  point->set_y(point->get_y() / divisor);
  return self;
}

template <> struct mrubypp::converter<Point> {
  static mrb_value to_mrb(mrb_state *mrb, const Point &var) {
    mrb_value obj = mrb_obj_value(
        mrb_data_object_alloc(mrb, mrb->object_class, new Point(var),
                              &mrubypp::bind_class<Point>::data_type));
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

TEST_CASE("bind_class", "[class]") {
  mrubypp::engine engine;
  mrubypp::bind_class<Point>(engine.get_mrb(), "Point")
      .def_constructor<int, int>()
      .def_method("add", &Point::add)
      .def_class_method("none", &Point::none)
      .def_property("x", &Point::get_x, &Point::set_x)
      .def_property("y", &Point::get_y, &Point::set_y)
      .def_native("div", point_native_div, MRB_ARGS_REQ(1));

  engine.load(R"(
    def test_method()
      p = Point.new(3, 4)
      p.add(p)
      return p
    end
    def test_property()
      p = Point.new(3, 4)
      p.x = 10
      p.y = p.y + p.y
      return p
    end
    def test_native()
      p = Point.new(4, 8)
      p.div(2)
      return p
    end
    def test_class_method()
      return Point::none()
    end
  )");

  SECTION("test_method") {
    auto point_result = engine.call<Point>("test_method");
    REQUIRE(point_result.get_x() == 6);
    REQUIRE(point_result.get_y() == 8);
  }

  SECTION("test_property") {
    auto point_result = engine.call<Point>("test_property");
    REQUIRE(point_result.get_x() == 10);
    REQUIRE(point_result.get_y() == 8);
  }

  SECTION("test_native") {
    auto point_result = engine.call<Point>("test_native");
    REQUIRE(point_result.get_x() == 2);
    REQUIRE(point_result.get_y() == 4);
  }

  SECTION("test_class_method") {
    auto result = engine.call<int>("test_class_method");
    REQUIRE(result == 1);
  }
}
