//
// Created by ZekeXiao on 2025/10/18.
//
//
// Created by ZekeXiao on 2025/10/18.
//

#include <catch2/catch_all.hpp>

#include "engine.h"

TEST_CASE("none args call", "[engine]") {
  engine engine;
  engine.load(R"(
      def get_1()
        1
      end
  )");

  auto b = engine.call<int>("get_1");
  REQUIRE(b == 1);
}

TEST_CASE("args call", "[engine]") {
  engine engine;
  engine.load(R"(
      def sort_and_get_first(a)
        a.sort!
        a[0]
      end
  )");

  std::vector<int> a{3, 1, 2};
  auto b = engine.call<int>("sort_and_get_first", a);
  REQUIRE(b == 1);
}

TEST_CASE("call benchmark", "[!benchmark]") {
  engine engine;
  engine.load(R"(
      def get_same(a)
        return a
      end
  )");

  BENCHMARK("call and return") {
    auto b = engine.call<int>("get_same", 1);
    REQUIRE(b == 1);
  };
}
