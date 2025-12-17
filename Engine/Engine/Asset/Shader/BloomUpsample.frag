#version 460 core

in vec2 varying_tex_coords;

out vec4 out_color;

/* This texture needs:
 * 1) The wrapping mode to be set to clamp-to-edge.
 * 2) The filtering to be set to bilinear filtering. */
#pragma driven
uniform sampler2D uniform_tex_source;

#pragma slider( 0, 0.1, logarithmic, "%.3f texels" )
uniform float uniform_sample_radius;

void main()
{
    /* Upsampling filter is a tent filter that is 3x3:
     *
     *        | 1 2 1 |                   a b c
     * 1/16 x | 2 4 2 | corresponds to    d e f
     *        | 1 2 1 |                   g h i
     * 
     * We also rescale each tap down by a radius in order to account for the fact that upsampling from a smaller image to a bigger one
     * means a simple offset of 1 texel actually means a big jump, skpping texels in the original source texture.
     */

    vec4 a = texture( uniform_tex_source, varying_tex_coords + vec2( -uniform_sample_radius, +uniform_sample_radius ) );
    vec4 b = texture( uniform_tex_source, varying_tex_coords + vec2(  0,                     +uniform_sample_radius ) );
    vec4 c = texture( uniform_tex_source, varying_tex_coords + vec2( +uniform_sample_radius, +uniform_sample_radius ) );
    vec4 d = texture( uniform_tex_source, varying_tex_coords + vec2( -uniform_sample_radius,  0                     ) );
    vec4 e = texture( uniform_tex_source, varying_tex_coords + vec2(  0,                      0                     ) );
    vec4 f = texture( uniform_tex_source, varying_tex_coords + vec2( +uniform_sample_radius,  0                     ) );
    vec4 g = texture( uniform_tex_source, varying_tex_coords + vec2( -uniform_sample_radius, -uniform_sample_radius ) );
    vec4 h = texture( uniform_tex_source, varying_tex_coords + vec2(  0,                     -uniform_sample_radius ) );
    vec4 i = texture( uniform_tex_source, varying_tex_coords + vec2( +uniform_sample_radius, -uniform_sample_radius ) );

    out_color =
        e * 0.25 +
        ( b + d + f + h ) * 0.125 +
        ( a + c + g + i ) * 0.0625;
}
