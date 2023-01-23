add_library(gn-example-lib example_lib.cpp example_lib.h)
target_link_libraries(gn-example-lib PUBLIC gn-static)

function(gn_add_graphics_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
    target_compile_definitions(${_NAME} PUBLIC GN_EXAMPLE_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(gn_add_compute_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
    target_compile_definitions(${_NAME} PUBLIC GN_EXAMPLE_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

gn_add_graphics_example(gn_example_hello_world basic/hello_world/hello_world.cpp)
gn_add_graphics_example(gn_example_hello_triangle basic/hello_triangle/hello_triangle.cpp)
gn_add_compute_example(gn_example_hello_compute compute_basic/hello_compute/hello_compute.cpp)