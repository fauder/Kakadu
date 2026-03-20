#pragma once

// Engine Includes.
#include "RHI/Texture.h"

// std Includes.
#include <unordered_map>

namespace Kakadu
{
	/* Forward declarations: */
	class Renderer;

	/* Singleton. */
	class BuiltinTextures
	{
		friend class Renderer;

	public:
		static RHI::Texture* Get( const std::string& name );

		static RHI::Texture* CreateSingleTexelTexture( const Color4& texel, const std::string& name,
													   std::optional< RHI::Texture::ImportSettings > import_settings = std::nullopt );
		static RHI::Texture* CreateSingleTexelTexture( const std::array< unsigned char, 4 > texel, const std::string& name,
													   std::optional< RHI::Texture::ImportSettings > import_settings = std::nullopt );

	private:
		static void Initialize();

	private:
		static std::unordered_map< std::string, RHI::Texture* > TEXTURE_MAP;
	};
}
