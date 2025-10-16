// Engine Includes
#include "Intersect.h"

namespace Engine::Math
{
    /* Liang-Barsky algorithm.
     * Returns t-enter and t-exit values calculated. */
    Vector2 ClipLineAgainstRect( const Vector2 p1, const Vector2 p2, const Rect& rect, bool is_infinite_line )
    {
        /* Edges are mapped to indices as:
         * 0: Left
         * 1: Right
         * 2: Bottom
         * 3: Top */

        constexpr float infinity = TypeTraits< float >::Infinity();

        float t_enter = is_infinite_line ? -infinity : 0;
        float t_exit  = is_infinite_line ? +infinity : 1;
        const float p_values[ 4 ] = { p1.X() - p2.X(), p2.X() - p1.X(), p1.Y() - p2.Y(), p2.Y() - p1.Y() };
        const float q_values[ 4 ] =
        {
            p1.X() - rect.origin_x,
            rect.origin_x + rect.width - p1.X(),
            p1.Y() - rect.origin_y,
            rect.origin_y + rect.height - p1.Y()
        };

        for( int i = 0; i < 4; i++ )
        {
            float pk = p_values[ i ];
            float qk = q_values[ i ];

            if( pk == 0 )
            {
                if( qk < 0 )
                    /* Line is completely outside the rect. Discard => return point at infinity to signal this. */
                    return Vector2( infinity, infinity );

                continue; // Avoid possible divide by zero and also qk > 0 means the line is parallel to this edge anyway.
            }

            const float t = qk / pk;

            if( pk < 0 )
                t_enter = Max( t_enter, t );
            else if( pk > 0 )
                t_exit = Min( t_exit, t );
        }

        if( t_enter > t_exit )
            /* Line is completely outside the rect. Discard => return point at infinity to signal this. */
            return Vector2( infinity, infinity );

        return Vector2( t_enter, t_exit );
    }

    /* Liang-Barsky algorithm.
     * Modifies the passed in position vectors in-place. */
    void ClipLineAgainstRect_InPlace( Vector2& p1, Vector2& p2, const Rect& rect, bool is_infinite_line )
    {
        Vector2 start = p1;
        Vector2 end   = p2;

        Vector2 t_values = ClipLineAgainstRect( p1, p2, rect, is_infinite_line );

        if( std::isinf( t_values[ 0 ] ) || std::isinf( t_values[ 1 ] ) )
        {
            return;
        }

        p1 = Lerp( start, end, t_values[ 0 ] );
        p2 = Lerp( start, end, t_values[ 1 ] );
    }
}