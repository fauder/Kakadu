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

	static const auto& UVs()
	{
		static auto uvs = CircleTemplate::UVs();

		return uvs;
	}

	static const auto& Normals()
	{
		static auto normals = CircleTemplate::Normals();

		return normals;
	}

	static const auto& Tangents()
	{
		static auto tangents = CircleTemplate::Tangents();

		return tangents;
	}

	static const auto& Bitangents()
	{
		static auto bitangents = CircleTemplate::Bitangents();

		return bitangents;
	}
}
