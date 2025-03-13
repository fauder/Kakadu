#version 460 core

in vec3 varying_tex_coords;

out vec4 out_color;

uniform samplerCube uniform_tex;

void main()
{
	out_color = texture( uniform_tex, varying_tex_coords );
}
