#version 460 core
#extension GL_ARB_shading_language_include : require

in vec2 varying_tex_coords;

out vec4 out_color;

void main()
{
	out_color = vec4( varying_tex_coords.rg, 0.0, 1.0f );
}