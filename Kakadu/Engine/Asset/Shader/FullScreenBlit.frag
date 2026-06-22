#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

out vec4 out_color;

#pragma driven
uniform sampler2D uniform_tex;

void main()
{
    out_color = texture( uniform_tex, gl_FragCoord.xy / _INTRINSIC_VIEWPORT_SIZE );
}
