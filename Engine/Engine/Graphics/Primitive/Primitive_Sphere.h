#pragma once

// Project Includes.
#include "Primitive_UV_Sphere.hpp"

namespace Engine::Primitive::Indexed::Sphere
{
	static const auto& Positions()
	{
		static auto positions = UVSphereTemplate::Positions();

		return positions;
	}

	static const auto& Indices()
	{
		static auto indices = UVSphereTemplate::Indices();

		return indices;
	}

	constexpr auto UVs()
	{
		return UVSphereTemplate::UVs();
	}
}
