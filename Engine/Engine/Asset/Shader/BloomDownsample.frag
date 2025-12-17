#version 460 core

in vec2 varying_tex_coords;

out vec4 out_color;

/* This texture needs:
 * 1) The wrapping mode to be set to clamp-to-edge.
 * 2) The filtering to be set to bilinear filtering. */
#pragma driven
uniform sampler2D uniform_tex_source;
#pragma driven
uniform ivec2 uniform_source_resolution;

void main()
{
    /* The sampling pattern is:
     *
     * a b c
     *  j k 
     * d e f
     *  l m
     * g h i
     *
     * where e is the current pixel.
     * j, k, l, m are the immediate diagonal neighbours and get 0.5 total weight.
     * The rest of the 4 squares that are overlapping get 0.125 each:
     *      top-left     square a, b, d, e
     *      top-right    square b, c, e, f
     *      bottom-left  square d, e, g, h
     *      bottom-right square e, f, h, i
     *
     * Based on this, we can see that e is 1/4th of the overlapping 4 squares and therefore gets 0.125 weight on its own.
     * Similarly, b, d, f, h occur half as much and a, c, g, i occur quarter as much.
     * j, k, l, m have a total of 0.5 weight so each gets 0.125.
     */

    float delta_u = 1.0 / uniform_source_resolution.x;
    float delta_v = 1.0 / uniform_source_resolution.y;

    vec4 a = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u, +2 * delta_v ) );
    vec4 b = texture( uniform_tex_source, varying_tex_coords + vec2(  0 * delta_u, +2 * delta_v ) );
    vec4 c = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u, +2 * delta_v ) );
    vec4 d = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u,  0 * delta_v ) );
    vec4 e = texture( uniform_tex_source, varying_tex_coords + vec2(  0 * delta_u,  0 * delta_v ) );
    vec4 f = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u,  0 * delta_v ) );
    vec4 g = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u, -2 * delta_v ) );
    vec4 h = texture( uniform_tex_source, varying_tex_coords + vec2(  0 * delta_u, -2 * delta_v ) );
    vec4 i = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u, -2 * delta_v ) );
    vec4 j = texture( uniform_tex_source, varying_tex_coords + vec2( -1 * delta_u, +1 * delta_v ) );
    vec4 k = texture( uniform_tex_source, varying_tex_coords + vec2( +1 * delta_u, +1 * delta_v ) );
    vec4 l = texture( uniform_tex_source, varying_tex_coords + vec2( -1 * delta_u, -1 * delta_v ) );
    vec4 m = texture( uniform_tex_source, varying_tex_coords + vec2( +1 * delta_u, -1 * delta_v ) );

    out_color =
        e * 0.125 +
        ( b + d + f + h ) * 0.0625 +
        ( a + c + g + i ) * 0.03125 +
        ( j + k + l + m ) * 0.125;
}
