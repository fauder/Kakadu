// Engine Includes.
#include "RHI.h"
#include "ShaderType.h"

namespace Kakadu::RHI
{
	i32 ShaderTypeID( const ShaderType shader_type )
	{
		constexpr std::array< i32, ( i32 )ShaderType::_Count_ > shader_type_identifiers
		{
			GL_VERTEX_SHADER,
			GL_GEOMETRY_SHADER,
			GL_FRAGMENT_SHADER
		};

		return shader_type_identifiers[ ( i32 )shader_type ];
	}
}