#pragma once

// Project Includes.
#include "Primitive_Cylinder.hpp"

namespace Engine::Primitive::Indexed::Cylinder
{
	header_function const auto& Positions()
	{
		local_persist auto positions = CylinderTemplate::Positions();

		return positions;
	}

	header_function const auto& Indices()
	{
		local_persist auto indices = CylinderTemplate::Indices();

		return indices;
	}

	header_function const auto& UVs()
	{
		local_persist auto uvs = CylinderTemplate::UVs();

		return uvs;
	}

	constexpr auto Normals()
	{
		return CylinderTemplate::Normals();
	}

	constexpr auto Tangents()
	{
		return CylinderTemplate::Tangents();
	}

	constexpr auto Bitangents()
	{
		return CylinderTemplate::Bitangents();
	}
}
