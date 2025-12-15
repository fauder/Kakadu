#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

#pragma feature BLOOM

out vec4 out_color;

uniform sampler2D uniform_tex_color;
#ifdef BLOOM
uniform sampler2D uniform_tex_bloom;
uniform float uniform_bloom_intensity; /* _hint_normalized_percentage_logarithmic */
#endif

uniform float uniform_exposure;

void main()
{
    out_color = texture( uniform_tex_color, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE );

#ifdef BLOOM
    vec4 bloom = texture( uniform_tex_bloom, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE ) * uniform_bloom_intensity;

    out_color += bloom;
#endif

    // Exposure tone-mapping:
    out_color.rgb = vec3( 1.0 ) - exp( -out_color.rgb * uniform_exposure );

    out_color.a = 1.0f;
}
