#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <array>

namespace Kakadu::RHI
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
		constexpr std::array< const char*, ( i32 )ShaderType::_Count_ > shader_type_identifiers
		{
			"Vertex",
			"Geometry",
			"Fragment"
		};

		return shader_type_identifiers[ ( i32 )shader_type ];
	}

	constexpr const char* ShaderTypeString_Uppercase( const ShaderType shader_type )
	{
		constexpr std::array< const char*, ( i32 )ShaderType::_Count_ > shader_type_identifiers
		{
			"VERTEX",
			"GEOMETRY",
			"FRAGMENT"
		};

		return shader_type_identifiers[ ( i32 )shader_type ];
	}

	i32 ShaderTypeID( const ShaderType shader_type );
}
