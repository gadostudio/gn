gn_add_graphics_example(gn_example_vertex_buffer vertex_buffer.cpp)
gn_add_shader_example(
    TARGET gn_example_vertex_buffer
    GLSL_VERTEX vertex_buffer.vert
    GLSL_FRAGMENT vertex_buffer.frag)