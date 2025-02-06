// Engine Includes.
#include "GLLogger.h"
#include "Texture.h"
#include "Core/ServiceLocator.h"
#include "Core/Assertion.h"

namespace Engine
{
/*
 * Utility functions:
 */

	int InternalFormat( const Texture::Format format )
	{
		switch( format )
		{
			case Texture::Format::R:				return GL_RED;
			case Texture::Format::RG:				return GL_RG;
			case Texture::Format::RGB:				return GL_RGB;
			case Texture::Format::RGBA:				return GL_RGBA;

			case Texture::Format::RGBA_16F:			return GL_RGBA16F;
			case Texture::Format::RGBA_32F:			return GL_RGBA32F;

			case Texture::Format::SRGB:				return GL_SRGB;
			case Texture::Format::SRGBA:			return GL_SRGB_ALPHA;

			case Texture::Format::DEPTH_STENCIL:	return GL_DEPTH24_STENCIL8;
			case Texture::Format::DEPTH:			return GL_DEPTH_COMPONENT;
			case Texture::Format::STENCIL:			return GL_STENCIL_INDEX;

			default:
				throw std::logic_error( "PixelDataFormat(): Unknown pixel data format encountered!" );
				break;
		}
	};

	GLenum PixelDataFormat( const Texture::Format format )
	{
		switch( format )
		{
			case Texture::Format::R:				return GL_RED;
			case Texture::Format::RG:				return GL_RG;
			case Texture::Format::RGB:				return GL_RGB;
			case Texture::Format::RGBA:				return GL_RGBA;

			case Texture::Format::RGBA_16F:			return GL_RGBA;
			case Texture::Format::RGBA_32F:			return GL_RGBA;
			
			case Texture::Format::SRGB:				return GL_RGB;
			case Texture::Format::SRGBA:			return GL_RGBA;

			case Texture::Format::DEPTH_STENCIL:	return GL_DEPTH_STENCIL;
			case Texture::Format::DEPTH:			return GL_DEPTH_COMPONENT;
			case Texture::Format::STENCIL:			return GL_STENCIL_INDEX;

			default:
				throw std::logic_error( "PixelDataFormat(): Unknown pixel data format encountered!" );
				break;
		}
	}

	GLenum PixelDataType( const Texture::Format format )
	{
		switch( format )
		{
			default:								return GL_UNSIGNED_BYTE;

			case Texture::Format::RGBA_16F:			return GL_HALF_FLOAT;
			case Texture::Format::RGBA_32F:			return GL_FLOAT;

			case Texture::Format::DEPTH_STENCIL:	return GL_UNSIGNED_INT_24_8;
			case Texture::Format::DEPTH:			return GL_UNSIGNED_INT;
			case Texture::Format::STENCIL:			return GL_UNSIGNED_BYTE;
		}
	}

/* 
 * Texture
 */

	Texture::Texture()
		:
		id( {} ),
		size( ZERO_INITIALIZATION ),
		type( TextureType::None ),
		name( "<defaulted>" ),
		sample_count( 0 ),
		format( Format::NOT_ASSIGNED )
	{}

	/* Allocate-only constructor (no data). */
	Texture::Texture( const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const int width, const int height,
					  const Wrapping wrap_u, const Wrapping wrap_v,
					  const Color4 border_color,
					  const Filtering min_filter, Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		sample_count( 0 ),
		format( format )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   ( GLenum )wrap_v );

		if( wrap_u == Wrapping::ClampToBorder || wrap_v == Wrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.Data() );

		glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), nullptr );

		/* No mip-map generation since there is no data yet. */

		Unbind();
	}

	/* Multi-sampled allocate-only constructor (no data). */
	Texture::Texture( const int sample_count,
					  const std::string_view multi_sampled_texture_name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const int width, const int height )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D_MultiSample ),
		name( multi_sampled_texture_name ),
		sample_count( sample_count ),
		format( format )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name + " (" + std::to_string( sample_count ) + " samples) " );
#endif // _DEBUG

		glTexImage2DMultisample( GL_TEXTURE_2D_MULTISAMPLE, sample_count, InternalFormat( format ), width, height, GL_TRUE );

		/* Multi-sampled textures do not support setting of any sampler state, including filtering & wrapping modes. */

		Unbind();
	}

	/* Cubemap allocate-only constructor (no data). */
	Texture::Texture( CubeMapConstructorTag tag,
					  const std::string_view name,
					  //const std::byte* data, This is omitted from this public constructor.
					  const Format format,
					  const int width, const int height,
					  const Wrapping wrap_u, const Wrapping wrap_v, const Wrapping wrap_w,
					  Color4 border_color,
					  const Filtering min_filter, const Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		sample_count( 0 ),
		format( format )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG


		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						  0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), nullptr );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );

		if( wrap_u == Wrapping::ClampToBorder || wrap_v == Wrapping::ClampToBorder || wrap_w == Wrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border_color.Data() );

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
		sample_count( std::exchange( donor.sample_count, 0 ) ),
		format( std::exchange( donor.format, Format::NOT_ASSIGNED ) )
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
		sample_count = std::exchange( donor.sample_count, 0 );
		format       = std::exchange( donor.format, Format::NOT_ASSIGNED );

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

	void Texture::Activate( const int slot ) const
	{
		glActiveTexture( GL_TEXTURE0 + slot );
		Bind();
	}

	void Texture::GenerateMipmaps() const
	{
		Bind();
		ASSERT_DEBUG_ONLY( type == TextureType::Texture2D );

		glGenerateMipmap( ( GLenum )type );
	}

/*
 * TEXTURE PRIVATE API
 */

	/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data. */
	Texture::Texture( const std::string_view name,
					  const std::byte* data,
					  const Format format, const int width, const int height,
					  const bool generate_mipmaps,
					  const Wrapping wrap_u, const Wrapping wrap_v,
					  const Color4 border_color,
					  const Filtering min_filter, const Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Texture2D ),
		name( name ),
		sample_count( 0 ),
		format( format )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,	   ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,	   ( GLenum )wrap_v );

		if( wrap_u == Wrapping::ClampToBorder || wrap_v == Wrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color.Data() );

		glTexImage2D( GL_TEXTURE_2D, 0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), data );

		if( generate_mipmaps )
			glGenerateMipmap( GL_TEXTURE_2D );

		Unbind();
	}

	/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data. */
	Texture::Texture( CubeMapConstructorTag tag,
					  const std::string_view name,
					  const std::array< const std::byte*, 6 >& cubemap_data_array,
					  const Format format, 
					  const int width, const int height,
					  const Wrapping wrap_u, const Wrapping wrap_v,	const Wrapping wrap_w,
					  const Color4 border_color,
					  const Filtering min_filter, const Filtering mag_filter )
		:
		id( {} ),
		size( width, height ),
		type( TextureType::Cubemap ),
		name( name ),
		sample_count( 0 ),
		format( format )
	{
		glGenTextures( 1, id.Address() );
		Bind();

#ifdef _DEBUG
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_TEXTURE, id.Get(), this->name );
#endif // _DEBUG

		for( auto i = 0; i < 6; i++ )
			glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
						  0, InternalFormat( format ), width, height, 0, PixelDataFormat( format ), PixelDataType( format ), cubemap_data_array[ i ] );

		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, ( GLenum )min_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, ( GLenum )mag_filter );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,	 ( GLenum )wrap_u );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,	 ( GLenum )wrap_v );
		glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,	 ( GLenum )wrap_w );

		if( wrap_u == Wrapping::ClampToBorder || wrap_v == Wrapping::ClampToBorder || wrap_w == Wrapping::ClampToBorder )
			glTexParameterfv( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, border_color.Data() );

		Unbind();
	}

	void Texture::Delete()
	{
		if( IsValid() )
		{
			glDeleteTextures( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Texture::Bind() const
	{
		glBindTexture( ( GLenum )type, id.Get() );
	}

	void Texture::Unbind() const
	{
		glBindTexture( ( GLenum )type, 0 );
	}
}
