#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Kakadu::Primitive::NonIndexed::Quad_FullScreen
{
	constexpr std::array< Vector3, 6 > Positions
	( {
		{ -1.0f, -1.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f },
		{ -1.0f, -1.0f, 0.0f },
		{ -1.0f,  1.0f, 0.0f }
	} );

	constexpr std::array< Vector2, 6 > UVs
	( {
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f }
	} );
}
