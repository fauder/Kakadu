#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Kakadu::Primitive::NonIndexed::Cube
{
	constexpr std::array< Vector3, 36 > Positions
	( {
		{ -0.5f, -0.5f, -0.5f }, // FRONT
		{  0.5f,  0.5f, -0.5f }, // FRONT
		{  0.5f, -0.5f, -0.5f }, // FRONT
		{  0.5f,  0.5f, -0.5f }, // FRONT
		{ -0.5f, -0.5f, -0.5f }, // FRONT
		{ -0.5f,  0.5f, -0.5f }, // FRONT

		{ -0.5f,  0.5f,  0.5f }, // BACK
		{  0.5f, -0.5f,  0.5f }, // BACK
		{  0.5f,  0.5f,  0.5f }, // BACK
		{  0.5f, -0.5f,  0.5f }, // BACK
		{ -0.5f,  0.5f,  0.5f }, // BACK
		{ -0.5f, -0.5f,  0.5f }, // BACK

		{ -0.5f, -0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f, -0.5f }, // LEFT
		{ -0.5f,  0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f,  0.5f }, // LEFT

		{  0.5f, -0.5f, -0.5f }, // RIGHT
		{  0.5f,  0.5f,  0.5f }, // RIGHT
		{  0.5f, -0.5f,  0.5f }, // RIGHT
		{  0.5f,  0.5f,  0.5f }, // RIGHT
		{  0.5f, -0.5f, -0.5f }, // RIGHT
		{  0.5f,  0.5f, -0.5f }, // RIGHT

		{ -0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f,  0.5f }, // BOTTOM
		{  0.5f, -0.5f,  0.5f }, // BOTTOM
		{ -0.5f, -0.5f,  0.5f }, // BOTTOM
		{ -0.5f, -0.5f, -0.5f }, // BOTTOM

		{ -0.5f,  0.5f, -0.5f }, // TOP
		{  0.5f,  0.5f,  0.5f }, // TOP
		{  0.5f,  0.5f, -0.5f }, // TOP
		{  0.5f,  0.5f,  0.5f }, // TOP
		{ -0.5f,  0.5f, -0.5f }, // TOP
		{ -0.5f,  0.5f,  0.5f }  // TOP
	} );

	constexpr std::array< Vector2, 36 > UVs
	( {
		{ 0.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 0.0f, 0.0f }, // FRONT
		{ 0.0f, 1.0f }, // FRONT

		{ 0.0f, 0.0f }, // BACK
		{ 1.0f, 1.0f }, // BACK
		{ 1.0f, 0.0f }, // BACK
		{ 1.0f, 1.0f }, // BACK
		{ 0.0f, 0.0f }, // BACK
		{ 0.0f, 1.0f }, // BACK

		{ 0.0f, 0.0f }, // LEFT
		{ 1.0f, 1.0f }, // LEFT
		{ 1.0f, 0.0f }, // LEFT
		{ 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f }, // LEFT
		{ 0.0f, 1.0f }, // LEFT

		{ 0.0f, 0.0f }, // RIGHT
		{ 1.0f, 1.0f }, // RIGHT
		{ 1.0f, 0.0f }, // RIGHT
		{ 1.0f, 1.0f }, // RIGHT
		{ 0.0f, 0.0f }, // RIGHT
		{ 0.0f, 1.0f }, // RIGHT

		{ 0.0f, 1.0f }, // BOTTOM
		{ 1.0f, 1.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM
		{ 0.0f, 0.0f }, // BOTTOM
		{ 0.0f, 1.0f }, // BOTTOM

		{ 0.0f, 0.0f }, // TOP
		{ 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f }, // TOP
		{ 1.0f, 1.0f }, // TOP
		{ 0.0f, 0.0f }, // TOP
		{ 0.0f, 1.0f }  // TOP
	} );

	constexpr std::array< Vector3, 36 > Normals
	( {
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT

		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK

		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT

		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT

		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM

		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }  // TOP
	} );

	constexpr std::array< Vector3, 36 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT

		{ +1.0f,  0.0f,  0.0f }, // BACK
		{ +1.0f,  0.0f,  0.0f }, // BACK
		{ +1.0f,  0.0f,  0.0f }, // BACK
		{ +1.0f,  0.0f,  0.0f }, // BACK
		{ +1.0f,  0.0f,  0.0f }, // BACK
		{ +1.0f,  0.0f,  0.0f }, // BACK

		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT

		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT

		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM

		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }  // TOP
	} );

	constexpr std::array< Vector3, 36 > Bitangents
	( {
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT

		{  0.0f, -1.0f,  0.0f }, // BACK
		{  0.0f, -1.0f,  0.0f }, // BACK
		{  0.0f, -1.0f,  0.0f }, // BACK
		{  0.0f, -1.0f,  0.0f }, // BACK
		{  0.0f, -1.0f,  0.0f }, // BACK
		{  0.0f, -1.0f,  0.0f }, // BACK

		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT

		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT

		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM

		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }  // TOP
	} );

	constexpr std::array< Vector4, 36 > VertexColors
	( {
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT

		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK

		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT

		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT

		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM

		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }  // TOP
	} );
}

namespace Kakadu::Primitive::Indexed::Cube
{
	constexpr std::array< Vector3, 24 > Positions
	( {
		{ -0.5f, -0.5f, -0.5f }, // FRONT
		{ -0.5f,  0.5f, -0.5f }, // FRONT
		{  0.5f,  0.5f, -0.5f }, // FRONT
		{  0.5f, -0.5f, -0.5f }, // FRONT

		{  0.5f, -0.5f,  0.5f }, // BACK
		{  0.5f,  0.5f,  0.5f }, // BACK
		{ -0.5f,  0.5f,  0.5f }, // BACK
		{ -0.5f, -0.5f,  0.5f }, // BACK

		{ -0.5f, -0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f,  0.5f }, // LEFT
		{ -0.5f,  0.5f, -0.5f }, // LEFT
		{ -0.5f, -0.5f, -0.5f }, // LEFT

		{  0.5f, -0.5f, -0.5f }, // RIGHT
		{  0.5f,  0.5f, -0.5f }, // RIGHT
		{  0.5f,  0.5f,  0.5f }, // RIGHT
		{  0.5f, -0.5f,  0.5f }, // RIGHT

		{ -0.5f, -0.5f,  0.5f }, // BOTTOM
		{ -0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f, -0.5f }, // BOTTOM
		{  0.5f, -0.5f,  0.5f }, // BOTTOM

		{ -0.5f,  0.5f, -0.5f }, // TOP
		{ -0.5f,  0.5f,  0.5f }, // TOP
		{  0.5f,  0.5f,  0.5f }, // TOP
		{  0.5f,  0.5f, -0.5f }  // TOP
	} );

	constexpr std::array< u32, 36 > Indices
	( {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,

		8, 9, 10,
		8, 10, 11,

		12, 13, 14,
		12, 14, 15,

		16, 17, 18,
		16, 18, 19,

		20, 21, 22,
		20, 22, 23
	} );

	constexpr std::array< Vector2, 24 > UVs
	( {
		/* UVs are the same for every face. */

		{ 0.0f, 0.0f }, // FRONT
		{ 0.0f, 1.0f }, // FRONT
		{ 1.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f }, // FRONT

		{ 0.0f, 0.0f }, // BACK
		{ 0.0f, 1.0f }, // BACK
		{ 1.0f, 1.0f }, // BACK
		{ 1.0f, 0.0f }, // BACK

		{ 0.0f, 0.0f }, // LEFT
		{ 0.0f, 1.0f }, // LEFT
		{ 1.0f, 1.0f }, // LEFT
		{ 1.0f, 0.0f }, // LEFT

		{ 0.0f, 0.0f }, // RIGHT
		{ 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f }, // RIGHT
		{ 1.0f, 0.0f }, // RIGHT

		{ 0.0f, 0.0f }, // BOTTOM
		{ 0.0f, 1.0f }, // BOTTOM
		{ 1.0f, 1.0f }, // BOTTOM
		{ 1.0f, 0.0f }, // BOTTOM

		{ 0.0f, 0.0f }, // TOP
		{ 0.0f, 1.0f }, // TOP
		{ 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f }  // TOP
	} );

	constexpr std::array< Vector3, 24 > Normals
	( {
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT
		{  0.0f,  0.0f, -1.0f }, // FRONT

		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK
		{  0.0f,  0.0f, +1.0f }, // BACK

		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT
		{ -1.0f,  0.0f,  0.0f }, // LEFT

		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT
		{ +1.0f,  0.0f,  0.0f }, // RIGHT

		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM
		{  0.0f, -1.0f,  0.0f }, // BOTTOM

		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }, // TOP
		{  0.0f, +1.0f,  0.0f }  // TOP
	} );

	constexpr std::array< Vector3, 24 > Tangents
	( {
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT
		{ +1.0f,  0.0f,  0.0f }, // FRONT

		{ -1.0f,  0.0f,  0.0f }, // BACK
		{ -1.0f,  0.0f,  0.0f }, // BACK
		{ -1.0f,  0.0f,  0.0f }, // BACK
		{ -1.0f,  0.0f,  0.0f }, // BACK

		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT
		{  0.0f,  0.0f, -1.0f }, // LEFT

		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT
		{  0.0f,  0.0f, +1.0f }, // RIGHT

		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM
		{ +1.0f,  0.0f,  0.0f }, // BOTTOM

		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }, // TOP
		{ +1.0f,  0.0f,  0.0f }  // TOP
	} );

	constexpr std::array< Vector3, 24 > Bitangents
	( {
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT
		{  0.0f, +1.0f,  0.0f }, // FRONT

		{  0.0f, +1.0f,  0.0f }, // BACK
		{  0.0f, +1.0f,  0.0f }, // BACK
		{  0.0f, +1.0f,  0.0f }, // BACK
		{  0.0f, +1.0f,  0.0f }, // BACK

		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT
		{  0.0f, +1.0f,  0.0f }, // LEFT

		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT
		{  0.0f, +1.0f,  0.0f }, // RIGHT

		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM
		{  0.0f,  0.0f, -1.0f }, // BOTTOM

		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }, // TOP
		{  0.0f,  0.0f, +1.0f }  // TOP
	} );

	constexpr std::array< Vector4, 24 > VertexColors
	( {
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT
		{ 1.0f, 0.0f, 0.0f, 1.0f }, // FRONT

		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK
		{ 0.0f, 1.0f, 0.0f, 1.0f }, // BACK

		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT
		{ 0.0f, 0.0f, 1.0f, 1.0f }, // LEFT

		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT
		{ 1.0f, 1.0f, 0.0f, 1.0f }, // RIGHT

		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM
		{ 0.0f, 1.0f, 1.0f, 1.0f }, // BOTTOM

		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }, // TOP
		{ 1.0f, 0.0f, 1.0f, 1.0f }  // TOP
	} );
}
