#pragma once

// Project Includes.
#include "Primitive_Sphere.hpp"

namespace Engine::Primitive::Indexed::Sphere
{
	static const auto& Positions()
	{
		static auto positions = SphereTemplate::Positions();

		return positions;
	}
}
