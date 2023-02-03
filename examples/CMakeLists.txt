add_library(gn-example-lib example_lib.cpp example_lib.h)
target_compile_features(gn-example-lib PUBLIC cxx_std_17)
target_link_libraries(gn-example-lib PUBLIC gn-static)

function(gn_add_graphics_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_compile_features(${_NAME} PUBLIC cxx_std_17)
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
    target_compile_definitions(${_NAME} PUBLIC GN_EXAMPLE_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(gn_add_compute_example _NAME _SOURCE)
    add_executable(${_NAME} ${_SOURCE})
    target_compile_features(${_NAME} PUBLIC cxx_std_17)
    target_link_libraries(${_NAME} PRIVATE gn-example-lib)
    target_compile_definitions(${_NAME} PUBLIC GN_EXAMPLE_SRC_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
endfunction()

function(gn_add_shader_example)
    set(_ARG_SHADER TARGET GLSL_VERTEX GLSL_FRAGMENT GLSL_COMPUTE)
    cmake_parse_arguments(SHADER "" "${_ARG_SHADER}" "" ${ARGN})

    #get_source_file_property(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_GLSL_VERTEX}" TARGET_DIRECTORY ${SHADER_TARGET})
    #message(${OUTPUT_DIR})

    if(SHADER_GLSL_COMPUTE)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_COMPUTE}.spv"
            COMMAND Vulkan::glslangValidator
            ARGS -S comp -V100 -o "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_COMPUTE}.spv" "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_GLSL_COMPUTE}"
            DEPENDS "${SHADER_GLSL_COMPUTE}"
            VERBATIM)
        set(SHADER_DEPENDENCIES "${SHADER_DEPENDENCIES};${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_COMPUTE}.spv")
    endif()

    if(SHADER_GLSL_VERTEX)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_VERTEX}.spv"
            COMMAND Vulkan::glslangValidator
            ARGS -S vert -V100 -o "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_VERTEX}.spv" "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_GLSL_VERTEX}"
            DEPENDS "${SHADER_GLSL_VERTEX}"
            VERBATIM)
        set(SHADER_DEPENDENCIES "${SHADER_DEPENDENCIES};${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_VERTEX}.spv")
    endif()

    if(SHADER_GLSL_FRAGMENT)
        add_custom_command(
            OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_FRAGMENT}.spv"
            COMMAND Vulkan::glslangValidator
            ARGS -S frag -V100 -o "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_FRAGMENT}.spv" "${CMAKE_CURRENT_SOURCE_DIR}/${SHADER_GLSL_FRAGMENT}"
            DEPENDS "${SHADER_GLSL_FRAGMENT}"
            VERBATIM)
        set(SHADER_DEPENDENCIES "${SHADER_DEPENDENCIES};${CMAKE_CURRENT_BINARY_DIR}/${SHADER_GLSL_FRAGMENT}.spv")
    endif()

    add_custom_target(
        ${SHADER_TARGET}_shader
        DEPENDS ${SHADER_DEPENDENCIES})

    add_dependencies(${SHADER_TARGET} ${SHADER_TARGET}_shader)
endfunction()

add_subdirectory("basic/hello_world")
add_subdirectory("basic/index_buffer")
add_subdirectory("basic/staging_buffer")
add_subdirectory("basic/triangle")
add_subdirectory("basic/vertex_buffer")
add_subdirectory("compute_basic/hello_compute")

# gn_add_graphics_example(gn_example_hello_world basic/hello_world/hello_world.cpp)
# gn_add_graphics_example(gn_example_triangle basic/triangle/triangle.cpp)
# gn_add_compute_example(gn_example_hello_compute compute_basic/hello_compute/hello_compute.cpp)