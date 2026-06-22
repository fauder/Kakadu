#version 460 core

in vec3 varying_vertex_color;

out vec4 out_color;

void main()
{
	out_color = vec4( varying_vertex_color, 1.0f );
}