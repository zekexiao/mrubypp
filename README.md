# mrubypp

mruby c++ header only library

## Installation
- install mruby
- add to cmake project, link `mrubypp` or copy *.h to you projects
- `cmake -DMRUBYPP_BUILD_TEST=ON .` build tests

## Usage
```
  #include "mrubypp.h"

  mrubypp engine;
  engine.load(R"(
      def sort_and_get_first(a)
        a.sort!
        a[0]
      end
  )");

  std::vector<int> a{3, 1, 2};
  auto b = engine.call<int>("sort_and_get_first", a);
  assert(b == 1);
```