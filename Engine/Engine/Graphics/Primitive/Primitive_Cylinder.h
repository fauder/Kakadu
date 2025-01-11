#pragma once

// Project Includes.
#include "Primitive_Cylinder.hpp"

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

	static const auto& UVs()
	{
		static auto uvs = CylinderTemplate::UVs();

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
