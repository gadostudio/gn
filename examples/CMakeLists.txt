add_library(gn-example-lib example_lib.cpp example_lib.h)
target_link_libraries(gn-example-lib PUBLIC gn-static)

function(gn_add_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
endfunction()

gn_add_example(gn-example-helloworld 01_helloworld.cpp)