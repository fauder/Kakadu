// Engine Includes.
#include "BuiltinTextures.h"
#include "Renderer.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Utility.hpp"
#include "Asset/AssetDirectoryPath.h"

#define FullTexturePath( file_path ) ENGINE_ASSET_PATH( "Texture/" file_path )

namespace Engine
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Texture* > BuiltinTextures::TEXTURE_MAP;

	Texture* BuiltinTextures::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( TEXTURE_MAP.contains( name ) && ( "Built-in texture with the name \"" + name + "\" was not found!" ).c_str() );

		return TEXTURE_MAP.find( name )->second;
	}

	Texture* BuiltinTextures::CreateSingleTexelTexture( const Color4& texel, const std::string& name,
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

	Texture* BuiltinTextures::CreateSingleTexelTexture( const std::array< unsigned char, 4 > texel, const std::string& name, 
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

	void BuiltinTextures::Initialize()
	{
		TEXTURE_MAP.try_emplace( "White",		CreateSingleTexelTexture( Color4::White(),		"White" ) );
		TEXTURE_MAP.try_emplace( "Gray",		CreateSingleTexelTexture( Color4::Gray( 0.5f ),	"Gray"	) );
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

		{
			/* Source: http://www.anisopteragames.com/how-to-fix-color-banding-with-dithering/. */
			static const char bayer_dither_matrix[] =
			{
				0, 32,  8, 40,  2, 34, 10, 42,   /* 8x8 Bayer ordered dithering  */
				48, 16, 56, 24, 50, 18, 58, 26,  /* pattern.  Each input pixel   */
				12, 44,  4, 36, 14, 46,  6, 38,  /* is scaled to the 0..63 range */
				60, 28, 52, 20, 62, 30, 54, 22,  /* before looking in this table */
				3, 35, 11, 43,  1, 33,  9, 41,   /* to determine the action.     */
				51, 19, 59, 27, 49, 17, 57, 25,
				15, 47,  7, 39, 13, 45,  5, 37,
				63, 31, 55, 23, 61, 29, 53, 21
			};

			TEXTURE_MAP.try_emplace( "Bayer Dither", AssetDatabase< Texture >::CreateAssetFromMemory( "Bayer Dither",
																									  reinterpret_cast< const std::byte* >( &bayer_dither_matrix ),
																									  sizeof( bayer_dither_matrix ) / sizeof( char ),
																									  true, // => using raw data instead of file contents.
																									  Texture::ImportSettings
																									  {
																										  .wrap_u     = Texture::Wrapping::Repeat,
																										  .wrap_v     = Texture::Wrapping::Repeat,
																										  .min_filter = Texture::Filtering::Nearest,
																										  .mag_filter = Texture::Filtering::Nearest,

																										  .flip_vertically  = false,
																										  .generate_mipmaps = false,

																										  .format = Texture::Format::R,
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
