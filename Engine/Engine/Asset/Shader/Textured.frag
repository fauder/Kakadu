#version 460 core
#pragma feature DISCARD_TRANSPARENT_FRAGMENTS

in vec2 varying_tex_coords;

out vec4 out_color;

uniform sampler2D uniform_tex;

void main()
{
	out_color = texture( uniform_tex, varying_tex_coords );
#ifdef DISCARD_TRANSPARENT_FRAGMENTS
	if( out_color.a < 0.1 )
		discard;
#endif
}