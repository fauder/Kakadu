#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

#pragma feature BLOOM

out vec4 out_color;

#pragma driven
uniform sampler2D uniform_tex_color;
#ifdef BLOOM
#pragma driven
uniform sampler2D uniform_tex_bloom;
#pragma slider( 0, 1, logarithmic, percentage )
uniform float uniform_bloom_intensity;
#endif

#pragma slider( -10, 10, "%.1f EV" )
uniform float uniform_exposure_ev;

void main()
{
    out_color = texture( uniform_tex_color, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE );

#ifdef BLOOM
    vec4 bloom = texture( uniform_tex_bloom, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE ) * uniform_bloom_intensity;

    out_color += bloom;
#endif

    // Exposure tone-mapping:
    float exposure_scale = exp2( uniform_exposure_ev );
    out_color.rgb = vec3( 1.0 ) - exp( -( out_color.rgb * exposure_scale ) );

    out_color.a = 1.0f;
}
