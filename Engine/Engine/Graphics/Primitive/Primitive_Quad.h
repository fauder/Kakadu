#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Quad
{
	constexpr std::array< Vector3, 6 > Positions
	( {
		{ -0.5f, -0.5f,  0.0f },
		{  0.5f,  0.5f,  0.0f },
		{  0.5f, -0.5f,  0.0f },
		{  0.5f,  0.5f,  0.0f },
		{ -0.5f, -0.5f,  0.0f },
		{ -0.5f,  0.5f,  0.0f }
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

	constexpr std::array< Vector3, 6 > Normals
	( {
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f }
	} );

	constexpr std::array< Vector3, 6 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f }
	} );
}

namespace Engine::Primitive::Indexed::Quad
{
	constexpr std::array< Vector3, 4 > Positions
	( {
		{ -0.5f, -0.5f,  0.0f },
		{ -0.5f,  0.5f,  0.0f },
		{  0.5f,  0.5f,  0.0f },
		{  0.5f, -0.5f,  0.0f }
	} );

	constexpr std::array< std::uint32_t, 6 > Indices
	( {
		0, 1, 2,
		0, 2, 3
	} );

	constexpr std::array< Vector2, 4 > UVs
	( {
		{ 0.0f, 0.0f },
		{ 0.0f, 1.0f },
		{ 1.0f, 1.0f },
		{ 1.0f, 0.0f }
	} );

	constexpr std::array< Vector3, 4 > Normals
	( {
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f },
		{  0.0f,  0.0f, -1.0f }
	} );

	constexpr std::array< Vector3, 4 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f },
		{ +1.0f,  0.0f,  0.0f }
	} );
}
