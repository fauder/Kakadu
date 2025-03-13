#version 460 core

out vec4 out_color;

uniform sampler2DMS uniform_tex;
uniform int uniform_sample_count;

void main()
{
    out_color = vec4( 0.0 );

    for( int i = 0; i < uniform_sample_count; ++i )
        out_color += texelFetch( uniform_tex, ivec2( gl_FragCoord.xy ), i );

    out_color /= float( uniform_sample_count ); // Average the samples.
}
