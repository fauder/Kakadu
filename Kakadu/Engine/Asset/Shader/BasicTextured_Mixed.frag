#version 460 core

in vec2 varying_uv;

out vec4 out_color;

uniform sampler2D uniform_texture_sampler_1;
uniform sampler2D uniform_texture_sampler_2;

void main()
{
	out_color = mix( texture( uniform_texture_sampler_1, varying_uv ), texture( uniform_texture_sampler_2, varying_uv ), 0.5 );
}