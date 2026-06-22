#pragma once

// Engine Includes.
#include "RHI/Shader.hpp"

// std Includes.
#include <unordered_map>

namespace Kakadu
{
	/* Forward declarations: */
	class Renderer;

	/* Singleton. */
	class BuiltinShaders
	{
		friend class Renderer;

	public:
		static RHI::Shader* Get( const std::string& name );

	private:
		static void Initialize( Renderer& renderer );

	private:
		static std::unordered_map< std::string, RHI::Shader > SHADER_MAP;
	};
}
