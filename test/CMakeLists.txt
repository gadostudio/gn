set(GN_TEST_SOURCES
    instance_test.cpp
    device_test.cpp
    test_driver.cpp)

add_executable(gn-test-d3d12 ${GN_TEST_SOURCES})
target_compile_definitions(gn-test-d3d12 PUBLIC GN_TEST_BACKEND_D3D12)
target_link_libraries(gn-test-d3d12 PRIVATE gn-static)

add_executable(gn-test-vulkan ${GN_TEST_SOURCES})
target_compile_definitions(gn-test-vulkan PUBLIC GN_TEST_BACKEND_VULKAN)
target_link_libraries(gn-test-vulkan PRIVATE gn-static ${GN_STATIC_DEPS})

add_executable(gn-barrier-test-vulkan barrier_conv_test.cpp)
target_compile_definitions(gn-barrier-test-vulkan PUBLIC GN_TEST_BACKEND_VULKAN)
target_link_libraries(gn-barrier-test-vulkan PRIVATE gn-static ${GN_STATIC_DEPS})

add_executable(gnsl-test-bootstrapper gnsl_test_bootstrapper.cpp)
target_link_libraries(gnsl-test-bootstrapper PRIVATE gnsl-static)
target_compile_definitions(gnsl-test-bootstrapper PUBLIC GNSL_TEST_CASE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/gnsl_test_case")