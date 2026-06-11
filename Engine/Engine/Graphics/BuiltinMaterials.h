#pragma once

// Engine Includes.
#include "Material.hpp"

// std Includes.
#include <unordered_map>

namespace Kakadu
{
	/* Forward declarations: */
	class Renderer;

	/* Singleton. */
	class BuiltinMaterials
	{
		friend class Renderer;

	public:
		static Material* Get( const std::string& name );

	private:
		static void Initialize();

	private:
		static std::unordered_map< std::string, Material > MATERIAL_MAP;
	};
}
