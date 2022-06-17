# CMakeList.txt : CMake project for gncxx, include source and define
# project specific logic here.
#
cmake_minimum_required(VERSION 3.8)

project("gncxx")

option(GN_ENABLE_VULKAN "Enable Vulkan Backend" ON)

add_library(gn INTERFACE)
set_property(TARGET gn PROPERTY CXX_STANDARD 17)
target_include_directories(gn
    INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
              $<INSTALL_INTERFACE:include>)

add_executable(gn-test
    test/expect_test.cpp
    test/impl_stub.cpp
    test/instance_test.cpp
    test/test_driver.cpp)
set_property(TARGET gn-test PROPERTY CXX_STANDARD 17)
target_link_libraries(gn-test PRIVATE gn)
