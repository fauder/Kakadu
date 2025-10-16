#pragma once

// Engine Includes.
#include "Vector.hpp"

namespace Engine::Math
{
	struct Rect
	{
        Rect( const Vector2 origin, const Vector2 extents )
            :
            origin( origin ),
            extents( extents )
        {}

        union
        {
            Vector2 origin;
            struct { float origin_x, origin_y; };
        };

        union
        {
            Vector2 extents;
            struct { float width, height; };
        };
	};
}
