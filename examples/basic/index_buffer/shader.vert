#version 450

layout(location = 0) in vec2 i_pos;
layout(location = 1) in vec4 i_color; // We are using normalized value, so it should

layout(location = 0) out vec4 o_color;

void main()
{
    o_color = i_color;
    gl_Position = vec4(i_pos, 0.0, 1.0);
}