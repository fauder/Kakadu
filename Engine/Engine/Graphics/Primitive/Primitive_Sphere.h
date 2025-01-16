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

	static const auto& Normals()
	{
		static auto normals = UVSphereTemplate::Normals();

		return normals;
	}

	static const auto& Tangents()
	{
		static auto tangents = UVSphereTemplate::Tangents();

		return tangents;
	}

	static const auto& Bitangents()
	{
		static auto bitangents = UVSphereTemplate::Bitangents();

		return bitangents;
	}
}
