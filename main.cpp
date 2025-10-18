#include <iostream>

#include <chrono>

#include "example/Point.h"
#include "mrubypp.h"
#include "mrubypp_bind_class.h"

#ifdef _WIN32
#include <Windows.h>
#include <assert.h>
#endif
int main() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
#endif
  mrubypp engine;
  engine.load(R"(
def add(a)
 a.sort!
 a[0]
end)");

  std::vector<int> a{3, 1, 2};
  static volatile int b = 0;
  std::chrono::steady_clock::time_point start =
      std::chrono::steady_clock::now();
  for (auto i = 0; i < 10000; ++i) {
    b = engine.call<int>("add", a);
    assert(b == 1);
  }
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
          .count();
  std::cout << "10000 ruby call duration: " << duration << "ms" << std::endl;

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

  // 调用test_point函数
  auto point_result = engine.call<Point>("test_point");
  std::cout << "returned point: [" << point_result.get_x() << ", "
            << point_result.get_y() << "]" << std::endl;

  return 0;
}
