#version 460 core

out vec4 out_color;

#pragma feature TONEMAPPING

uniform sampler2DMS uniform_texture_slot;
uniform int uniform_sample_count;

#ifdef TONEMAPPING
uniform float uniform_tonemapping_exposure;
#endif

void main()
{
    out_color = vec4( 0.0 );
    for( int i = 0; i < uniform_sample_count; ++i )
        out_color += texelFetch( uniform_texture_slot, ivec2( gl_FragCoord.xy ), i );

    out_color /= float( uniform_sample_count ); // Average the samples.

#ifdef TONEMAPPING
    // Exposure tone-mapping:
    out_color.rgb = vec3( 1.0 ) - exp( -out_color.rgb * uniform_tonemapping_exposure );
#endif
}
