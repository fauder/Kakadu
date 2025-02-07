#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

#pragma feature TONEMAPPING

out vec4 out_color;

uniform sampler2D uniform_texture_slot;

#ifdef TONEMAPPING
uniform float uniform_tonemapping_exposure;
#endif

void main()
{
    out_color = texture( uniform_texture_slot, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE );

#ifdef TONEMAPPING
    // Exposure tone-mapping:
    out_color.rgb = vec3( 1.0 ) - exp( -out_color.rgb * uniform_tonemapping_exposure );
#endif
}
