// Engine Includes.
#include "Texture.h"
#include "GLLogger.h"
#include "Core/ServiceLocator.h"

// Vendor/stb Includes.
#include "stb/stb_image.h"

// std Includes.
#include <array>
#include <execution>
#include <iostream>

namespace Engine
{
	constexpr int DESIRED_CHANNELS = 4;

	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		std::optional< Texture > maybe_texture;

		int width, height;
		int number_of_channels = -1;
		auto image_data = stbi_load( file_path.c_str(), &width, &height, &number_of_channels, DESIRED_CHANNELS );
		if( image_data )
		{
			maybe_texture = Texture( name, 
									 ( std::byte* )image_data, 
									 import_settings.format,
									 width, height, 
									 import_settings.generate_mipmaps,
									 import_settings.wrap_u, import_settings.wrap_v,
									 import_settings.border_color, 
									 import_settings.min_filter, import_settings.mag_filter );
		}
		else
		{
			std::cerr << R"(Texture ")" << name << R"(": Could not load image data from file: ")" << file_path << "\"\n";
			ServiceLocator< GLLogger >::Get().Error( R"(Texture ")" + std::string( name ) + R"(" Could not load image data from file: ")" + file_path + "\"\n" );
		}

		stbi_image_free( image_data );
		
		return maybe_texture;
	}

	std::optional< Texture > Texture::Loader::FromFile( const::std::string_view cubemap_name, const std::initializer_list< std::string > cubemap_file_paths,
														const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		/* OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		 *
		 * BUT: cube-map coordinate space has the inverse v behavior, so the image's should not be flipped. */
		stbi_set_flip_vertically_on_load( false ); // Override whatever is in import_settings.flip_vertically.

		std::array< stbi_uc*, 6 > image_data_array;

		int width, height;
		int number_of_channels = -1;

		bool error_encountered = false;

		constexpr std::array< std::uint8_t, 6 > indices = { 0, 1, 2, 3, 4, 5 };

		std::for_each( std::execution::par, indices.cbegin(), indices.cend(), [ & ]( std::uint8_t i )
		{
			const auto& file_path( ( cubemap_file_paths.begin() + i ) );

			image_data_array[ i ] = stbi_load( file_path->c_str(), &width, &height, &number_of_channels, DESIRED_CHANNELS );
			if( image_data_array[ i ] == nullptr )
			{
				std::cerr << R"(Cubemap texture ")" << cubemap_name << R"(": Could not load image data from file: ")" << *file_path << "\"\n";
				ServiceLocator< GLLogger >::Get().Error( R"(Cubemap texture ")" + std::string( cubemap_name ) + R"(": Could not load image data from file: ")" + *file_path + "\"\n" );
				stbi_image_free( image_data_array[ i ] );
				error_encountered = true;
			}
		} );

		if( error_encountered )
			return std::nullopt;

		std::optional< Texture > maybe_texture = Texture( CUBEMAP_CONSTRUCTOR,
														  cubemap_name,
														  ( const std::array< const std::byte*, 6 >& )image_data_array,
														  import_settings.format,
														  width,
														  height,
														  import_settings.border_color,
														  import_settings.min_filter,
														  import_settings.mag_filter );

		std::for_each( std::execution::par, indices.cbegin(), indices.cend(), [ & ]( std::uint8_t i )
		{
			stbi_image_free( image_data_array[ i ] );
		} );
		
		return maybe_texture;
	}

	/* 'data' argument here contains the parsed file contents that are still encoded and need to be decoded before actual use. */
	std::optional< Texture > Texture::Loader::FromFileBytes( const::std::string_view name,
															 const std::byte* data,
															 const int length,
															 const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		std::optional< Texture > maybe_texture;

		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( import_settings.flip_vertically );

		int width, height;
		int number_of_channels = -1;
		auto image_data = stbi_load_from_memory( ( stbi_uc* )data, length, &width, &height, &number_of_channels, DESIRED_CHANNELS );
		if( image_data )
		{
			maybe_texture = Texture( name, 
									 ( std::byte* )image_data, 
									 import_settings.format, 
									 width, height, 
									 import_settings.generate_mipmaps,
									 import_settings.wrap_u, import_settings.wrap_v,
									 import_settings.border_color,
									 import_settings.min_filter, import_settings.mag_filter );
		}
		else
		{
			std::cerr << R"(Texture ")" << name << R"(": Could not load image data from memory.")" << "\n";
			ServiceLocator< GLLogger >::Get().Error( R"(Texture ")" + std::string(name) + R"(": Could not load image data from memory.)" "\n" );
		}

		stbi_image_free( image_data );
		
		return maybe_texture;
	}

	/* 'data' here is already decoded and ready to be consumed directly. */
	std::optional< Texture > Texture::Loader::FromRawBytes( const::std::string_view name,
															const std::byte* data,
															const SizeType size,
															const ImportSettings& import_settings )
	{
		//auto& instance = Instance();

		std::optional< Texture > maybe_texture;

		maybe_texture = Texture( name,
								 data,
								 import_settings.format,
								 size.X(), size.Y(),
								 import_settings.generate_mipmaps,
								 import_settings.wrap_u, import_settings.wrap_v,
								 import_settings.border_color,
								 import_settings.min_filter, import_settings.mag_filter );
		return maybe_texture;
	}
}
