add_library(gn-example-lib example_lib.cpp example_lib.h)
target_link_libraries(gn-example-lib PUBLIC gn-static)

function(gn_add_graphics_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
endfunction()

function(gn_add_compute_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_link_libraries(${_NAME} PRIVATE gn-static)
endfunction()

gn_add_graphics_example(gn-example-helloworld 01_helloworld.cpp)

gn_add_compute_example(gn_example_compute_basic_hellocompute compute_basic/01_hellocompute.cpp)