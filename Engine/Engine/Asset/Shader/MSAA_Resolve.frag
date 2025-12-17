#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Math.glsl"

#pragma feature HDR_AWARE

#define SAMPLE_COUNT 4
#define SAMPLE_WEIGHT ( 1.0 / float( SAMPLE_COUNT ) )

out vec4 out_color;

#pragma driven
uniform sampler2DMS uniform_tex;

/* Below Tonemap operators are taken directly from Timothy Lottes' "Optimized Reversible Tonemapper for Resolve",
found here: https://gpuopen.com/learn/optimized-reversible-tonemapper-for-resolve/ */

// Apply this to tonemap linear HDR color "c" after a sample is fetched in the resolve.
// Note "c" 1.0 maps to the expected limit of low-dynamic-range monitor output.
vec3 Tonemap( vec3 c )
{
    return c / ( Max3( c.r, c.g, c.b ) + 1.0 );
}

// When the filter kernel is a weighted sum of fetched colors,
// it is more optimal to fold the weighting into the tonemap operation.
vec3 TonemapWithWeight( vec3 c, float w )
{
    return c * ( w / ( Max3( c.r, c.g, c.b ) + 1.0 ) );
}

// Apply this to restore the linear HDR color before writing out the result of the resolve.
vec3 TonemapInvert( vec3 c )
{
    return c / ( 1.0 - Max3( c.r, c.g, c.b ) );
}

void main()
{
    out_color = vec4( 0.0 );

#ifdef HDR_AWARE
    for( int i = 0; i < SAMPLE_COUNT; ++i )
    {
        vec4 sample_i = texelFetch( uniform_tex, ivec2( gl_FragCoord.xy ), i );
        out_color.rgb += TonemapWithWeight( sample_i.rgb, SAMPLE_WEIGHT );
        out_color.a   += SAMPLE_WEIGHT * sample_i.a;
    }

    out_color.rgb = TonemapInvert( out_color.rgb );
#else
    for( int i = 0; i < SAMPLE_COUNT; ++i )
        out_color += texelFetch( uniform_tex, ivec2( gl_FragCoord.xy ), i );

    out_color /= float( SAMPLE_COUNT );
#endif
}
