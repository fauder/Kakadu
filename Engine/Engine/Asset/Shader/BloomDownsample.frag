#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Light.glsl"

#pragma feature ANTI_FLICKER_COARSE
#pragma feature ANTI_FLICKER_FINE

#if defined( ANTI_FLICKER_COARSE ) && defined( ANTI_FLICKER_FINE )
    #error "Define only one: ANTI_FLICKER_COARSE or ANTI_FLICKER_FINE."
#endif

in vec2 varying_tex_coords;

out vec3 out_color;

/* This texture needs:
 * 1) The wrapping mode to be set to clamp-to-edge.
 * 2) The filtering to be set to bilinear filtering. */
#pragma driven
uniform sampler2D uniform_tex_source;
#pragma driven
uniform ivec2 uniform_source_resolution;

#if defined( ANTI_FLICKER_COARSE ) || defined( ANTI_FLICKER_FINE )
uniform uint uniform_mip_level;
#endif

vec3 KarisAverageOnBlock( vec3 sample_0, vec3 sample_1, vec3 sample_2, vec3 sample_3 )
{
    vec4 weights = 1.0 / ( 1.0 + vec4(
        MaxBrightness( sample_0 ),
        MaxBrightness( sample_1 ),
        MaxBrightness( sample_2 ),
        MaxBrightness( sample_3 ) ) );

    float sum_of_weights = weights[ 0 ] + weights[ 1 ] + weights[ 2 ] + weights[ 3 ];

    return
        ( sample_0 * weights[ 0 ] +
          sample_1 * weights[ 1 ] +
          sample_2 * weights[ 2 ] +
          sample_3 * weights[ 3 ] ) / sum_of_weights;
}

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

    vec3 a = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u, +2 * delta_v ) ).rgb;
    vec3 b = texture( uniform_tex_source, varying_tex_coords + vec2(            0, +2 * delta_v ) ).rgb;
    vec3 c = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u, +2 * delta_v ) ).rgb;
    vec3 d = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u,            0 ) ).rgb;
    vec3 e = texture( uniform_tex_source, varying_tex_coords + vec2(            0,            0 ) ).rgb;
    vec3 f = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u,            0 ) ).rgb;
    vec3 g = texture( uniform_tex_source, varying_tex_coords + vec2( -2 * delta_u, -2 * delta_v ) ).rgb;
    vec3 h = texture( uniform_tex_source, varying_tex_coords + vec2(            0, -2 * delta_v ) ).rgb;
    vec3 i = texture( uniform_tex_source, varying_tex_coords + vec2( +2 * delta_u, -2 * delta_v ) ).rgb;
    vec3 j = texture( uniform_tex_source, varying_tex_coords + vec2( -1 * delta_u, +1 * delta_v ) ).rgb;
    vec3 k = texture( uniform_tex_source, varying_tex_coords + vec2( +1 * delta_u, +1 * delta_v ) ).rgb;
    vec3 l = texture( uniform_tex_source, varying_tex_coords + vec2( -1 * delta_u, -1 * delta_v ) ).rgb;
    vec3 m = texture( uniform_tex_source, varying_tex_coords + vec2( +1 * delta_u, -1 * delta_v ) ).rgb;

#ifdef ANTI_FLICKER_COARSE
    if( uniform_mip_level == 0 )
    {
        // w components are reserved for weights below:
        vec4 block_top_left     = vec4( ( a + b + d + e ) * 0.25f, 0.0 );
        vec4 block_top_right    = vec4( ( b + c + e + f ) * 0.25f, 0.0 );
        vec4 block_bottom_left  = vec4( ( d + e + g + h ) * 0.25f, 0.0 );
        vec4 block_bottom_right = vec4( ( e + f + h + i ) * 0.25f, 0.0 );
    
        vec4 block_middle       = vec4( ( j + k + l + m ) * 0.25f, 0.0 );

        // Both karis and spatial weights are incorporated:
        block_top_left.w     = 0.125 / ( 1.0 + MaxBrightness( block_top_left.rgb ) );
        block_top_right.w    = 0.125 / ( 1.0 + MaxBrightness( block_top_right.rgb ) );
        block_bottom_left.w  = 0.125 / ( 1.0 + MaxBrightness( block_bottom_left.rgb ) );
        block_bottom_right.w = 0.125 / ( 1.0 + MaxBrightness( block_bottom_right.rgb ) );

        block_middle.w       = 0.5 / ( 1.0 + MaxBrightness( block_middle.rgb ) );

        out_color = (
            block_top_left.rgb     * block_top_left.w +
            block_top_right.rgb    * block_top_right.w +
            block_bottom_left.rgb  * block_bottom_left.w +
            block_bottom_right.rgb * block_bottom_right.w +
            block_middle.rgb       * block_middle.w ) / 
                ( block_top_left.w + block_top_right.w + block_bottom_left.w + block_bottom_right.w + block_middle.w );
            
        return;
    }
#endif // ANTI_FLICKER_COARSE

#ifdef ANTI_FLICKER_FINE
    /* This applies the partial Karis avg. on all 13 taps on the first mip of the downsampling chain, 
     * operating on 4x4 blocks (hence the name "partial").
     * Once the blocks themselves are averaged via Karis avg.,
     * the resulting color values are spatially averaged just as they are in the other downsampling passes. */

    if( uniform_mip_level == 0 )
    {
        vec3 karis_averaged_block_top_left     = KarisAverageOnBlock( a, b, d, e );
        vec3 karis_averaged_block_top_right    = KarisAverageOnBlock( b, c, e, f );
        vec3 karis_averaged_block_bottom_left  = KarisAverageOnBlock( d, e, g, h );
        vec3 karis_averaged_block_bottom_right = KarisAverageOnBlock( e, f, h, i );
        vec3 karis_averaged_block_middle       = KarisAverageOnBlock( j, k, l, m );

        out_color =
            ( karis_averaged_block_top_left    +
              karis_averaged_block_top_right   +
              karis_averaged_block_bottom_left +
              karis_averaged_block_bottom_right ) * 0.125 +
            karis_averaged_block_middle * 0.5;

        return;
    }
#endif //ANTI_FLICKER_FINE

    out_color =
        ( b + d + f + h ) * 0.0625 +
        ( a + c + g + i ) * 0.03125 +
        ( j + k + l + m + e ) * 0.125;
}
