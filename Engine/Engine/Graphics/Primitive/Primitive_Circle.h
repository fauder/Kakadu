#pragma once

// Project Includes.
#include "Primitive_Circle.hpp"

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

	static const auto& UVs()
	{
		static auto uvs = CircleTemplate::UVs();

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
