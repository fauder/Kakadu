#pragma once

// Engine Includes.
#include "Texture.h"

// std Includes.
#include <unordered_map>

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	/* Singleton. */
	class BuiltinTextures
	{
		friend class Renderer;

	public:
		static Texture* Get( const std::string& name );

		static Texture* CreateSingleTexelTexture( const Color4& texel, const std::string& name, 
												  std::optional< Texture::ImportSettings > import_settings = std::nullopt );
		static Texture* CreateSingleTexelTexture( const std::array< unsigned char, 4 > texel, const std::string& name,
												  std::optional< Texture::ImportSettings > import_settings = std::nullopt );

	private:
		static void Initialize();

	private:
		static std::unordered_map< std::string, Texture* > TEXTURE_MAP;
	};
}
