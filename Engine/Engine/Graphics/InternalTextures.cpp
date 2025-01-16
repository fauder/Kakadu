// Engine Includes.
#include "InternalTextures.h"
#include "Renderer.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Utility.hpp"
#include "Asset/Texture/InternalTextureDirectoryPath.h"


#define FullTexturePath( file_path ) std::string( Engine::TEXTURE_SOURCE_DIRECTORY_WITH_SEPARATOR ) + file_path
// Too lazy to get the above macro working constexpr. And frankly, unnecessary.

namespace Engine
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Texture* > InternalTextures::TEXTURE_MAP;

	Texture* InternalTextures::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( TEXTURE_MAP.contains( name ) && ( "Built-in texture with the name \"" + name + "\" was not found!" ).c_str() );

		return TEXTURE_MAP.find( name )->second;
	}

	void InternalTextures::Initialize()
	{
		{
			const unsigned char normal_map_texel[] = { 127, 127, 255, 255 };
			TEXTURE_MAP.try_emplace( "Normal Map", AssetDatabase< Texture >::CreateAssetFromMemory( "Default Normal Map",
																									reinterpret_cast< const std::byte* >( &normal_map_texel ),
																									1,
																									true, // => using raw data instead of file contents.
																									Texture::ImportSettings
																									{
																										.wrap_u           = Texture::Wrapping::ClampToEdge,
																										.wrap_v           = Texture::Wrapping::ClampToEdge,
																										.min_filter       = Texture::Filtering::Nearest,
																										.mag_filter       = Texture::Filtering::Nearest,
																										.flip_vertically  = false,
																										.is_sRGB          = false,
																										.generate_mipmaps = false,
																									} ) );
		}

		{
			const unsigned char texel[] = { 255, 255, 255, 255 };
			TEXTURE_MAP.try_emplace( "White", AssetDatabase< Texture >::CreateAssetFromMemory( "White",
																							   reinterpret_cast< const std::byte* >( &texel ),
																							   1,
																							   true, // => using raw data instead of file contents.
																							   Texture::ImportSettings
																							   {
																								   .wrap_u           = Texture::Wrapping::ClampToEdge,
																								   .wrap_v           = Texture::Wrapping::ClampToEdge,
																								   .min_filter       = Texture::Filtering::Nearest,
																								   .mag_filter       = Texture::Filtering::Nearest,
																								   .flip_vertically  = false,
																								   .is_sRGB          = true,
																								   .generate_mipmaps = false,
																							   } ) );
		}

		TEXTURE_MAP.try_emplace( "Missing", AssetDatabase< Texture >::CreateAssetFromFile( "Missing", FullTexturePath( "missing_texture.jpg" ),
																						   Texture::ImportSettings
																						   {
																							   .wrap_u = Texture::Wrapping::Repeat,
																							   .wrap_v = Texture::Wrapping::Repeat
																						   } ) );

		TEXTURE_MAP.try_emplace( "UV Test", AssetDatabase< Texture >::CreateAssetFromFile( "UV Test", FullTexturePath( "uv_test.png" ),
																						   Texture::ImportSettings
																						   {
																							   .wrap_u = Texture::Wrapping::Repeat,
																							   .wrap_v = Texture::Wrapping::Repeat
																						   } ) );
	}
}
