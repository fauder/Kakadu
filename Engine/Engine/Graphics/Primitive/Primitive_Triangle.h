#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Triangle
{
	constexpr std::array< Vector3, 3 > Positions
	( {
		{ -0.5f, -0.5f,  0.0f },
		{  0.5f,  0.5f,  0.0f },
		{  0.5f, -0.5f,  0.0f }
	} );

	constexpr std::array< Vector2, 3 > UVs
	( {
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	} );

	constexpr std::array< Vector3, 3 > Normals
	( {
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f }
	} );

	constexpr std::array< Vector3, 3 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f }
	} );
}
