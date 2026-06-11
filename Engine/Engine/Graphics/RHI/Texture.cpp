// Engine Includes.
#include "RHI.h"
#include "Capabilities.h"
#include "DebugLabel.h"
#include "GLLabelPrefixes.h"
#include "Texture.h"
#include "Core/ServiceLocator.hpp"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	Texture::Texture()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		type( TextureType::None ),
		name( "<defaulted>" ),
		import_settings{ .format = Format::NOT_ASSIGNED }
	{
	}

	/* Allocate-only constructor (no data). */
	Texture::Texture( const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const i32 width, const i32 height,
					  const TextureWrapping wrap_u, const TextureWrapping wrap_v,
					  const Color4 border_color,
					  const TextureFiltering min_filter, TextureFiltering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		import_settings
		{
			.wrap_u       = wrap_u,
			.wrap_v       = wrap_v,
			.border_color = border_color,
			.min_filter   = min_filter,
			.mag_filter   = mag_filter,
			.format       = DetermineActualFormat( format )
		}
	{
		glGenTextures( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			DebugLabel::Set( GL_TEXTURE, id.id, GL_LABEL_PREFIX_TEXTURE + this->name );
#endif // _EDITOR

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilteringToGLEnum( min_filter ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilteringToGLEnum( mag_filter ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   TextureWrappingToGLEnum( wrap_u ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   TextureWrappingToGLEnum( wrap_v ) );

		if( wrap_u == TextureWrapping::ClampToBorder || wrap_v == TextureWrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data );

		glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), nullptr );

		/* No mip-map generation since there is no data yet. */

		Unbind();
	}

	/* Multi-sampled allocate-only constructor (no data). */
	Texture::Texture( const std::string_view multi_sample_texture_name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const u8 sample_count,
					  const i32 width, const i32 height )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D_MultiSample ),
		name( multi_sample_texture_name ),
		import_settings
		{
			.format = DetermineActualFormat( format ),
			.msaa = MSAA{ Capabilities::QueryMSAASupport( format, sample_count ) ? sample_count : u8( 1 ) }
		}
	{
		glGenTextures( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			DebugLabel::Set( GL_TEXTURE, id.id, GL_LABEL_PREFIX_TEXTURE + this->name +
							 ( name.find( "MSAA" ) == std::string::npos
							   ? "(MSAA " + std::to_string( sample_count ) + "x)"
							   : "" ) );
#endif // _EDITOR

		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sample_count, InternalFormat( format ), width, height, GL_TRUE );

		/* Multi-sampled textures do not support setting of any sampler state, including filtering & wrapping modes. */

		Unbind();
	}

	/* Cubemap allocate-only constructor (no data).
	 * No wrapping options for cubemaps as all three axes will default to clamp-to-edge, which is the only sensible option. */
	Texture::Texture( CubeMapConstructorTag tag,
					  const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const i32 width, const i32 height,
					  Color4 border_color,
					  const TextureFiltering min_filter, const TextureFiltering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		import_settings
		{
			.border_color = border_color,
			.min_filter   = min_filter,
			.mag_filter   = mag_filter,
			.format       = DetermineActualFormat( format )
		}
	{
		glGenTextures( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			DebugLabel::Set( GL_TEXTURE, id.id, GL_LABEL_PREFIX_TEXTURE + this->name );
#endif // _EDITOR


		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						  0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), nullptr );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilteringToGLEnum( min_filter ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilteringToGLEnum( mag_filter ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );

		Unbind();
	}

	Texture::Texture( Texture&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		size( std::move( donor.size ) ),
		type( std::move( donor.type ) ),
#ifdef _DEBUG
		name( std::exchange( donor.name, "<moved-from>" ) ),
#else
		name( std::move( donor.name ) ),
#endif // _DEBUG
		import_settings( std::exchange( donor.import_settings, { .format = Format::NOT_ASSIGNED } ) )
	{
	}

	Texture& Texture::operator=( Texture&& donor )
	{
		Delete();

		id   = std::exchange( donor.id, {} );
		size = std::move( donor.size );
		type = std::move( donor.type );
#ifdef _DEBUG
		name = std::exchange( donor.name, "<moved-from>" );
#else
		name = std::move( donor.name );
#endif // _DEBUG
		import_settings = std::exchange( donor.import_settings, { .format = Format::NOT_ASSIGNED } );

		return *this;
	}

	Texture::~Texture()
	{
		Delete();
	}

	void Texture::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Texture::Activate( const i32 slot ) const
	{
		glActiveTexture( GL_TEXTURE0 + slot );
		Bind();
	}

	void Texture::GenerateMipmaps() const
	{
		Bind();
		ASSERT_DEBUG_ONLY( type == TextureType::Texture2D );

		glGenerateMipmap( TextureTypeToGLEnum( type ) );
	}

	i32 Texture::InternalFormat( const Texture::Format format )
	{
		switch( format )
		{
			case Format::R:				return GL_RED;
			case Format::RG:			return GL_RG;
			case Format::RGB:			return GL_RGB;
			case Format::RGBA:			return GL_RGBA;

			case Format::RGBA_16F:		return GL_RGBA16F;
			case Format::RGBA_32F:		return GL_RGBA32F;
			case Format::R11G11B10F:	return GL_R11F_G11F_B10F;

			case Format::SRGB:			return GL_SRGB;
			case Format::SRGBA:			return GL_SRGB_ALPHA;

			case Format::DEPTH_STENCIL:	return GL_DEPTH24_STENCIL8;
			case Format::DEPTH:			return GL_DEPTH_COMPONENT;
			case Format::STENCIL:		return GL_STENCIL_INDEX;

			default:
				throw std::logic_error( "InternalFormat(): Unknown pixel data format encountered!" );
				break;
		}
	};

	GLenum Texture::PixelDataFormat( const Texture::Format format )
	{
		switch( format )
		{
			case Format::R:				return GL_RED;
			case Format::RG:			return GL_RG;
			case Format::RGB:			return GL_RGB;
			case Format::RGBA:			return GL_RGBA;

			case Format::RGBA_16F:		return GL_RGBA;
			case Format::RGBA_32F:		return GL_RGBA;

			case Format::R11G11B10F:	return GL_RGB;

			case Format::SRGB:			return GL_RGB;
			case Format::SRGBA:			return GL_RGBA;

			case Format::DEPTH_STENCIL:	return GL_DEPTH_STENCIL;
			case Format::DEPTH:			return GL_DEPTH_COMPONENT;
			case Format::STENCIL:		return GL_STENCIL_INDEX;

			default:
				throw std::logic_error( "PixelDataFormat(): Unknown pixel data format encountered!" );
				break;
		}
	}

	GLenum Texture::PixelDataType( const Texture::Format format )
	{
		switch( format )
		{
			default:					return GL_UNSIGNED_BYTE;

			case Format::RGBA_16F:		return GL_HALF_FLOAT;
			case Format::RGBA_32F:		return GL_FLOAT;
			case Format::R11G11B10F:	return GL_FLOAT;

			case Format::DEPTH_STENCIL:	return GL_UNSIGNED_INT_24_8;
			case Format::DEPTH:			return GL_UNSIGNED_INT;
			case Format::STENCIL:		return GL_UNSIGNED_BYTE;
		}
	}

/*
 * TEXTURE PRIVATE API
 */

	/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data.
	 * If data argument is nullptr, this is essentially the same as the public allocate-only constructor. */
	Texture::Texture( const std::string_view name,
					  const std::byte* data,
					  const Format format, const i32 width, const i32 height,
					  const bool generate_mipmaps,
					  const TextureWrapping wrap_u, const TextureWrapping wrap_v,
					  const Color4 border_color,
					  const TextureFiltering min_filter, const TextureFiltering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		import_settings
		{
			.wrap_u           = wrap_u,
			.wrap_v           = wrap_v,
			.border_color     = border_color,
			.min_filter       = min_filter,
			.mag_filter       = mag_filter,
			.generate_mipmaps = true,
			.format           = DetermineActualFormat( format )
		}
	{
		glGenTextures( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			DebugLabel::Set( GL_TEXTURE, id.id, GL_LABEL_PREFIX_TEXTURE + this->name );
#endif // _EDITOR

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, TextureFilteringToGLEnum( min_filter ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, TextureFilteringToGLEnum( mag_filter ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   TextureWrappingToGLEnum( wrap_u ) );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   TextureWrappingToGLEnum( wrap_v ) );

		if( wrap_u == TextureWrapping::ClampToBorder || wrap_v == TextureWrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.data );

		glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), data );

		if( data && generate_mipmaps )
			glGenerateMipmap( GL_TEXTURE_2D );

		Unbind();
	}

	/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data.
	 * No wrapping options for cubemaps as all three axes will default to clamp-to-edge, which is the only sensible option. */
	Texture::Texture( CubeMapConstructorTag tag,
					  const std::string_view name,
					  const std::array< const std::byte*, 6 >& cubemap_data_array,
					  const Format format, 
					  const i32 width, const i32 height,
					  const Color4 border_color,
					  const TextureFiltering min_filter, const TextureFiltering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		import_settings
		{
			.border_color     = border_color,
			.min_filter       = min_filter,
			.mag_filter       = mag_filter,
			.format           = DetermineActualFormat( format )
		}
	{
		glGenTextures( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			DebugLabel::Set( GL_TEXTURE, id.id, GL_LABEL_PREFIX_TEXTURE + this->name );
#endif // _EDITOR

		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						  0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), cubemap_data_array[ i ] );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, TextureFilteringToGLEnum( min_filter ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, TextureFilteringToGLEnum( mag_filter ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 TextureWrappingToGLEnum( TextureWrapping::ClampToEdge ) );

		Unbind();
	}

	void Texture::Delete()
	{
		if( IsValid() )
		{
#ifdef _EDITOR
			std::cout << "Deleting Texture id #" << id.id << ": " << name << ".\n";
#endif // _EDITOR

			glDeleteTextures( 1, &id.id );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Texture::Bind() const
	{
		glBindTexture( TextureTypeToGLEnum( type ), id.id );
	}

	void Texture::Unbind() const
	{
		glBindTexture( TextureTypeToGLEnum( type ), 0 );
	}
}
