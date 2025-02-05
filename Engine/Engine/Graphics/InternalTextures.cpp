// Engine Includes.
#include "InternalTextures.h"
#include "Renderer.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Utility.hpp"
#include "Asset/AssetDirectoryPath.h"

#define FullTexturePath( file_path ) std::string( Engine::ASSET_SOURCE_DIRECTORY_WITH_SEPARATOR ) + R"(Texture\)" + file_path
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

	Texture* InternalTextures::CreateSingleTexelTexture( const Color4& texel, const std::string& name,
														 std::optional< Texture::ImportSettings > import_settings )
	{
		return CreateSingleTexelTexture( std::array< unsigned char, 4 >
										 {
											 unsigned char( texel.R() * 255.0f ),
											 unsigned char( texel.G() * 255.0f ),
											 unsigned char( texel.B() * 255.0f ),
											 unsigned char( texel.A() * 255.0f )
										 },
										 name,
										 import_settings );
	}

	Texture* InternalTextures::CreateSingleTexelTexture( const std::array< unsigned char, 4 > texel, const std::string& name, 
														 std::optional< Texture::ImportSettings > import_settings )
	{
		if( not import_settings.has_value() )
		{
			import_settings = Texture::ImportSettings
			{
				.wrap_u           = Texture::Wrapping::ClampToEdge,
				.wrap_v           = Texture::Wrapping::ClampToEdge,
				.min_filter       = Texture::Filtering::Nearest,
				.mag_filter       = Texture::Filtering::Nearest,
				.flip_vertically  = false,
				.generate_mipmaps = false
			};
		}

		return AssetDatabase< Texture >::CreateAssetFromMemory( name,
																reinterpret_cast< const std::byte* >( &texel ),
																1,
																true, // => using raw data instead of file contents.
																*import_settings );
	}

	void InternalTextures::Initialize()
	{
		TEXTURE_MAP.try_emplace( "White",		CreateSingleTexelTexture( Color4::White(),		"White" ) );
		TEXTURE_MAP.try_emplace( "Black",		CreateSingleTexelTexture( Color4::Black(),		"Black" ) );
		TEXTURE_MAP.try_emplace( "Normal Map",	CreateSingleTexelTexture( std::array< unsigned char, 4 >{ 127, 127, 255, 255 }, "Default Normal Map",
																		  Texture::ImportSettings
																		  {
																			  .wrap_u = Texture::Wrapping::ClampToEdge,
																			  .wrap_v = Texture::Wrapping::ClampToEdge,
																			  .min_filter = Texture::Filtering::Nearest,
																			  .mag_filter = Texture::Filtering::Nearest,
																			  .flip_vertically = false,
																			  .generate_mipmaps = false,

																			  .format = Texture::Format::RGBA
																		  } ) );


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
