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

	static const auto& Indices()
	{
		static auto indices = SphereTemplate::Indices();

		return indices;
	}

	constexpr auto UVs()
	{
		return SphereTemplate::UVs();
	}
}
