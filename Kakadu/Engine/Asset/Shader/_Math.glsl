#ifndef _MATH_GLSL
#define _MATH_GLSL

#include "_Intrinsic_Other.glsl"

float Max3( float x, float y, float z )
{
    return max( x, max( y, z ) );
}

float Max3( vec3 v )
{
    return max( v.x, max( v.y, v.z ) );
}

vec3 Pow( vec3 v, float p )
{
    return vec3( pow( v.x, p ), pow( v.y, p ), pow( v.z, p ) );
}

/* Liang-Barsky algorithm.
 * Returns t-enter and t-exit values calculated. */
vec2 ClipLineAgainstViewport( const vec2 p1, const vec2 p2, bool is_infinite_line )
{
    /* Edges are mapped to indices as:
     * 0: Left
     * 1: Right
     * 2: Bottom
     * 3: Top */

    const float infinity = 1.0 / 0.0;

    float t_enter = is_infinite_line ? -infinity : 0;
    float t_exit  = is_infinite_line ? +infinity : 1;
    const float p_values[ 4 ] = { p1.x - p2.x, p2.x - p1.x, p1.y - p2.y, p2.y - p1.y };
    /* x_min and y_min are always 0 since the viewport is created at 0,0 in the first place. */
    const float q_values[ 4 ] = { p1.x, _INTRINSIC_VIEWPORT_SIZE.x - p1.x, p1.y, _INTRINSIC_VIEWPORT_SIZE.y - p1.y };

    for( int i = 0; i < 4; i++ )
    {
        #define pk p_values[ i ]
        #define qk q_values[ i ]

        if( abs( pk ) < 1e-8 )
        {
            if( qk < 0 )
                /* Line is completely outside the viewport. Discard => return point at infinity to signal this, without breaking the math at client site. */
                return vec2( infinity, infinity );

            continue; // Avoid possible divide by zero and also qk > 0 means the line is parallel to this edge anyway.
        }

        const float t = qk / pk;

        if( pk < 0 )
            t_enter = max( t_enter, t );
        else if( pk > 0 )
            t_exit = min( t_exit, t );
    }

    if( t_enter > t_exit )
        /* Line is completely outside the viewport. Discard => return point at infinity to signal this, without breaking the math at client site. */
        return vec2( infinity, infinity );

    return vec2( t_enter, t_exit );
}

void ClipLineAgainstViewport_InPlace( inout vec2 p1, inout vec2 p2, bool is_infinite_line )
{
    vec2 start = p1;
    vec2 end   = p2;

    vec2 t_values = ClipLineAgainstViewport( p1, p2, is_infinite_line );

    if( any( isinf( t_values ) ) )
        // Entirely outside: do not modify anything.
        return;

    p1 = mix( start, end, t_values[ 0 ] );
    p2 = mix( start, end, t_values[ 1 ] );
}

#endif // _MATH_GLSL