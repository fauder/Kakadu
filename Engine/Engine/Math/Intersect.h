#pragma once

// Engine Includes.
#include "Rect.h"
#include "Vector.hpp"

namespace Engine::Math
{
    /* Liang-Barsky algorithm.
     * Returns t-enter and t-exit values calculated. */
    Vector2 ClipLineAgainstRect( const Vector2 p1, const Vector2 p2, const Rect& rect, bool is_infinite_line );

    /* Liang-Barsky algorithm.
     * Modifies the passed in position vectors in-place. */
    void ClipLineAgainstRect_InPlace( Vector2& p1, Vector2& p2, const Rect& rect, bool is_infinite_line );
}
