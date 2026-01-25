#ifndef _LIGHT_GLSL
#define _LIGHT_GLSL

#include "_Color.glsl"
#include "_Math.glsl"

/*
 * Luminance:
 */

float RGB_To_Luminance( vec3 color )
{
    return dot( color, vec3( 0.2126f, 0.7152f, 0.0722f ) );
}

float MaxBrightness( vec3 color )
{
    return Max3( color );
}

#endif // _LIGHT_GLSL
