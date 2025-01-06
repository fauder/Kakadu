#pragma once

// Project Includes.
#include "Primitive_Circle.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::Indexed::Circle
{
	static const auto& Positions()
	{
		static auto positions = CircleTemplate::Positions();

		return positions;
	}

	static const auto& Indices()
	{
		static auto indices = CircleTemplate::Indices();

		return indices;
	}
}
