#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

in vec2 varying_tex_coords;

out vec4 out_color;

/* This texture needs:
 * 1) The wrapping mode set to clamp-to-edge.
 * 2) The filtering set to bilinear. */
#pragma driven
uniform sampler2D uniform_tex_source;

void main()
{
    /* Upsampling filter is a tent filter that is 3x3:
     *
     *        | 1 2 1 |                   a b c
     * 1/16 x | 2 4 2 | corresponds to    d e f
     *        | 1 2 1 |                   g h i
     * 
     * The delt-uvs can simply be derived as usual; 1 over the resolution of the texture being sampled,
     * which in this case is the lower resolution mip level i that is being upsampled to mip level i+1.
     * Since the viewport size is given as an intrinsic uniform, we can simply use half that as the resolution.
     */

    vec2 delta_uv = 1.0 / ( 0.5 * _INTRINSIC_VIEWPORT_SIZE );

    vec4 a = texture( uniform_tex_source, varying_tex_coords + vec2( -delta_uv.s,   +delta_uv.t ) );
    vec4 b = texture( uniform_tex_source, varying_tex_coords + vec2(  0,            +delta_uv.t ) );
    vec4 c = texture( uniform_tex_source, varying_tex_coords + vec2( +delta_uv.s,   +delta_uv.t ) );
    vec4 d = texture( uniform_tex_source, varying_tex_coords + vec2( -delta_uv.s,   0           ) );
    vec4 e = texture( uniform_tex_source, varying_tex_coords + vec2(  0,            0           ) );
    vec4 f = texture( uniform_tex_source, varying_tex_coords + vec2( +delta_uv.s,   0           ) );
    vec4 g = texture( uniform_tex_source, varying_tex_coords + vec2( -delta_uv.s,   -delta_uv.t ) );
    vec4 h = texture( uniform_tex_source, varying_tex_coords + vec2(  0,            -delta_uv.t ) );
    vec4 i = texture( uniform_tex_source, varying_tex_coords + vec2( +delta_uv.s,   -delta_uv.t ) );

    out_color =
        e * 0.25 +
        ( b + d + f + h ) * 0.125 +
        ( a + c + g + i ) * 0.0625;
}
