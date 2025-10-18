cmake_minimum_required(VERSION 3.16)

project(mrubypp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MRUBYPP_BUILD_TEST "Build Catch2 Test" ON)

if (MSVC)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_definitions(NOMINMAX)
endif ()

add_library(mrubypp INTERFACE
        mrubypp.h
        mrubypp_converters.h
        mrubypp_arena_guard.h
        mrubypp_bind_class.h)


include_directories(${CMAKE_CURRENT_LIST_DIR})

if (${MRUBYPP_BUILD_TEST})
    if (NOT mruby_ROOT)
        message(NOTICE "[mrubypp] Using default mruby build")
        set(mruby_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/deps/mruby")
    endif ()


    add_subdirectory(deps/Catch2)

    enable_testing()

    add_executable(mrubypp_test
            test/test_class.cpp
            test/test_engine.cpp)


    target_include_directories(mrubypp_test PUBLIC "${mruby_ROOT}/include")
    target_link_directories(mrubypp_test PUBLIC "${mruby_ROOT}/lib")
    target_link_libraries(mrubypp_test PUBLIC
            libmruby ws2_32.lib wsock32.lib ws2_32.lib)

    target_link_libraries(mrubypp_test PRIVATE mrubypp Catch2::Catch2WithMain)
    add_test(NAME mrubypp_test COMMAND mrubypp_test)
endif ()

include(GNUInstallDirs)
install(TARGETS mrubypp
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
