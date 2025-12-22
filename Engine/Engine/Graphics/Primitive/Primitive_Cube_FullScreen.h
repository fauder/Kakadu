#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::NonIndexed::Cube_FullScreen
{
	header_function constexpr std::array< Vector3, 36 > Positions
	( {
		{ -1.0f, -1.0f, -1.0f }, // FRONT
		{  1.0f,  1.0f, -1.0f }, // FRONT
		{  1.0f, -1.0f, -1.0f }, // FRONT
		{  1.0f,  1.0f, -1.0f }, // FRONT
		{ -1.0f, -1.0f, -1.0f }, // FRONT
		{ -1.0f,  1.0f, -1.0f }, // FRONT

		{ -1.0f,  1.0f,  1.0f }, // BACK
		{  1.0f, -1.0f,  1.0f }, // BACK
		{  1.0f,  1.0f,  1.0f }, // BACK
		{  1.0f, -1.0f,  1.0f }, // BACK
		{ -1.0f,  1.0f,  1.0f }, // BACK
		{ -1.0f, -1.0f,  1.0f }, // BACK

		{ -1.0f, -1.0f,  1.0f }, // LEFT
		{ -1.0f,  1.0f, -1.0f }, // LEFT
		{ -1.0f, -1.0f, -1.0f }, // LEFT
		{ -1.0f,  1.0f, -1.0f }, // LEFT
		{ -1.0f, -1.0f,  1.0f }, // LEFT
		{ -1.0f,  1.0f,  1.0f }, // LEFT

		{  1.0f, -1.0f, -1.0f }, // RIGHT
		{  1.0f,  1.0f,  1.0f }, // RIGHT
		{  1.0f, -1.0f,  1.0f }, // RIGHT
		{  1.0f,  1.0f,  1.0f }, // RIGHT
		{  1.0f, -1.0f, -1.0f }, // RIGHT
		{  1.0f,  1.0f, -1.0f }, // RIGHT

		{ -1.0f, -1.0f, -1.0f }, // BOTTOM
		{  1.0f, -1.0f, -1.0f }, // BOTTOM
		{  1.0f, -1.0f,  1.0f }, // BOTTOM
		{  1.0f, -1.0f,  1.0f }, // BOTTOM
		{ -1.0f, -1.0f,  1.0f }, // BOTTOM
		{ -1.0f, -1.0f, -1.0f }, // BOTTOM

		{ -1.0f,  1.0f, -1.0f }, // TOP
		{  1.0f,  1.0f,  1.0f }, // TOP
		{  1.0f,  1.0f, -1.0f }, // TOP
		{  1.0f,  1.0f,  1.0f }, // TOP
		{ -1.0f,  1.0f, -1.0f }, // TOP
		{ -1.0f,  1.0f,  1.0f }  // TOP
	} );
}
