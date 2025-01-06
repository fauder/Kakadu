#pragma once

// Project Includes.
#include "Primitive_Cylinder.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::Indexed::Cylinder
{
	static const auto& Positions()
	{
		static auto positions = CylinderTemplate::Positions();

		return positions;
	}

	static const auto& Indices()
	{
		static auto indices = CylinderTemplate::Indices();

		return indices;
	}
}
