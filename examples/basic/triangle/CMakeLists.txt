gn_add_graphics_example(gn_example_triangle triangle.cpp)
gn_add_shader_example(
    TARGET gn_example_triangle
    GLSL_VERTEX triangle.vert
    GLSL_FRAGMENT triangle.frag)