#pragma once

// Project Includes.
#include "Primitive_UV_Sphere.hpp"

namespace Engine::Primitive::Indexed::Sphere
{
	header_function const auto& Positions()
	{
		local_persist auto positions = UVSphereTemplate::Positions();

		return positions;
	}

	header_function const auto& Indices()
	{
		local_persist auto indices = UVSphereTemplate::Indices();

		return indices;
	}

	constexpr auto UVs()
	{
		return UVSphereTemplate::UVs();
	}

	header_function const auto& Normals()
	{
		local_persist auto normals = UVSphereTemplate::Normals();

		return normals;
	}

	header_function const auto& Tangents()
	{
		local_persist auto tangents = UVSphereTemplate::Tangents();

		return tangents;
	}

	header_function const auto& Bitangents()
	{
		local_persist auto bitangents = UVSphereTemplate::Bitangents();

		return bitangents;
	}
}
