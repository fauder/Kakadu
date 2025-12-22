#pragma once

// Project Includes.
#include "Primitive_Circle_Template.hpp"

namespace Engine::Primitive::Indexed::Circle
{
	header_function const auto& Positions()
	{
		local_persist auto positions = CircleTemplate::Positions();

		return positions;
	}

	header_function const auto& Indices()
	{
		local_persist auto indices = CircleTemplate::Indices();

		return indices;
	}

	header_function const auto& UVs()
	{
		local_persist auto uvs = CircleTemplate::UVs();

		return uvs;
	}

	constexpr auto Normals()
	{
		return CircleTemplate::Normals();
	}

	constexpr auto Tangents()
	{
		return CircleTemplate::Tangents();
	}

	constexpr auto Bitangents()
	{
		return CircleTemplate::Bitangents();
	}
}
