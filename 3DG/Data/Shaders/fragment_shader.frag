#version 460 core

in vec4 vertex_color;
layout(location = 0) out vec4 fragment_colour;

void main(void)
{
	fragment_colour = vertex_color;
}

