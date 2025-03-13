#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

out vec4 out_color;

uniform sampler2D uniform_tex;

uniform float uniform_exposure;

void main()
{
    out_color = texture( uniform_tex, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE );

    // Exposure tone-mapping:
    out_color.rgb = vec3( 1.0 ) - exp( -out_color.rgb * uniform_exposure );

    out_color.a = 1.0f;
}
