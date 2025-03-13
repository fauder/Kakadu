#version 460 core

out vec4 out_color;

uniform sampler2D uniform_tex;

void main()
{
	out_color = texture( uniform_tex, gl_FragCoord.xy );
    float average = 0.2126 * out_color.r + 0.7152 * out_color.g + 0.0722 * out_color.b;
    out_color = vec4( average, average, average, 1.0 );
}
