#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Kakadu::Primitive::NonIndexed::Triangle
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

	// W components are set to +1.0f. Handedness doesn't change for primitive meshes constructed in-engine.
	constexpr std::array< Vector4, 3 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f, +1.0f },
		{ +1.0f,  0.0f,  0.0f, +1.0f },
		{ +1.0f,  0.0f,  0.0f, +1.0f }
	} );
}
