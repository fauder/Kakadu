#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <array>

namespace Engine
{
	enum class ShaderType
	{
		Vertex,
		Geometry,
		Fragment,

		_Count_
	};

	constexpr const char* ShaderTypeString( const ShaderType shader_type )
	{
		constexpr std::array< const char*, ( int )ShaderType::_Count_ > shader_type_identifiers
		{
			"Vertex",
			"Geometry",
			"Fragment"
		};

		return shader_type_identifiers[ ( int )shader_type ];
	}

	constexpr const char* ShaderTypeString_Uppercase( const ShaderType shader_type )
	{
		constexpr std::array< const char*, ( int )ShaderType::_Count_ > shader_type_identifiers
		{
			"VERTEX",
			"GEOMETRY",
			"FRAGMENT"
		};

		return shader_type_identifiers[ ( int )shader_type ];
	}

	constexpr int ShaderTypeID( const ShaderType shader_type )
	{
		constexpr std::array< int, ( int )ShaderType::_Count_ > shader_type_identifiers
		{
			GL_VERTEX_SHADER,
			GL_GEOMETRY_SHADER,
			GL_FRAGMENT_SHADER
		};

		return shader_type_identifiers[ ( int )shader_type ];
	}
}
