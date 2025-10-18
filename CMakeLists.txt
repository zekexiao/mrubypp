cmake_minimum_required(VERSION 3.16)

project(mrubypp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_definitions(NOMINMAX)
endif ()

add_executable(mrubypp main.cpp
        mrubypp.h
        mrubypp_converters.h
        mrubypp_arena_guard.h
        mrubypp_bind_class.h
        example/Point.cpp
        example/Point.h)


set(mruby_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/mruby")
target_include_directories(mrubypp PUBLIC "${mruby_ROOT}/include")
target_link_directories(mrubypp PUBLIC "${mruby_ROOT}/lib")
target_link_libraries(mrubypp PUBLIC
        libmruby
        ws2_32.lib wsock32.lib ws2_32.lib)

include_directories(${CMAKE_CURRENT_LIST_DIR})

include(GNUInstallDirs)
install(TARGETS mrubypp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
