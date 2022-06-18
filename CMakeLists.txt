# CMakeList.txt : CMake project for gncxx, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

project("gncxx")

option(GN_STATIC_BUILD "Build static library" ON)
option(GN_ENABLE_VULKAN "Enable Vulkan backend" ON)
option(GN_BUILD_EXAMPLES "Build examples (must build static library)" ON)
option(GN_BUILD_TESTS "Build tests (must build static library)" OFF)

add_library(gn INTERFACE)
set_property(TARGET gn PROPERTY CXX_STANDARD 17)
target_include_directories(gn
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)

if(GN_STATIC_BUILD OR GN_BUILD_EXAMPLES OR GN_BUILD_TESTS)
    add_library(gn-static STATIC src/gn_impl_stub.cpp)
    set_property(TARGET gn-static PROPERTY CXX_STANDARD 17)
    target_link_libraries(gn-static PUBLIC gn)

    if(GN_BUILD_EXAMPLES)
        # TODO: make examples
    endif()

    if(GN_BUILD_TESTS)
        add_executable(gn-test
            test/expect_test.cpp
            test/impl_stub.cpp
            test/instance_test.cpp
            test/test_driver.cpp)
        set_property(TARGET gn-test PROPERTY CXX_STANDARD 17)
        target_link_libraries(gn-test PRIVATE gn)
    endif()
endif()
