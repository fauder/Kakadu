// Engine Includes.
#include "Graphics.h"
#include "Framebuffer.h"
#include "GLLabelPrefixes.h"
#include "GLLogger.h"
#include "Core/Assertion.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Platform.h"
#include "Core/ServiceLocator.h"
#include "Core/MorphSystem.h"

namespace Engine
{
	Framebuffer::Framebuffer()
		:
		id( {} ),
		bind_point( BindPoint::Invalid ),
		size( ZERO_INITIALIZATION ),
		clear_color( Color4::Black() ),
		clear_depth_value( 1.0f ),
		clear_stencil_value( 0 ),
		name( "<default constructed FB>" ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
	}

	Framebuffer::Framebuffer( const Description& description )
		:
		id( {} ),
		bind_point( description.bind_point ),
		size( description.width_in_pixels, description.height_in_pixels ),
		msaa( description.msaa ),
		clear_color( Color4::Black() ),
		clear_depth_value( 1.0f ),
		clear_stencil_value( 0 ),
		name( description.name ),
		description( description ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
		Create();
	}

	Framebuffer::Framebuffer( Description&& description )
		:
		id( {} ),
		bind_point( description.bind_point ),
		size( description.width_in_pixels, description.height_in_pixels ),
		msaa( description.msaa ),
		clear_color( Color4::Black() ),
		clear_depth_value( 1.0f ),
		clear_stencil_value( 0 ),
		name( description.name ),
		description( std::move( description ) ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
		Create();
	}

	Framebuffer::Framebuffer( DefaultFramebuferConstructorTag )
		:
		id( 0 ),
		bind_point( BindPoint::Both ),
		size( Platform::GetFramebufferSizeInPixels() ),
		name( "Default Framebuffer" ),
		clear_targets( ClearTarget::ColorBuffer ),
		clear_color( Color4::Black() ),
		clear_depth_value( 1.0f ),
		clear_stencil_value( 0 ),
		color_attachment( nullptr ),
		depth_stencil_attachment( nullptr ),
		depth_attachment( nullptr ),
		stencil_attachment( nullptr )
	{
		/* Query attachments: */
		{
			int attachment;

			glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_DEPTH, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &attachment );
			if( attachment )
				clear_targets.Set( ClearTarget::DepthBuffer );

			glGetFramebufferAttachmentParameteriv( GL_FRAMEBUFFER, GL_STENCIL, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &attachment );
			if( attachment )
				clear_targets.Set( ClearTarget::StencilBuffer );
		}

		/* Query MSAA info: */
		{
			if( glIsEnabled( GL_MULTISAMPLE ) )
			{
				int queried_sample_count;
				glGetIntegerv( GL_SAMPLES, &queried_sample_count );

				if( queried_sample_count > 1 )
					msaa.sample_count = queried_sample_count;
			}
		}
	}

	Framebuffer::Framebuffer( Framebuffer&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		bind_point( std::exchange( donor.bind_point, BindPoint::Invalid ) ),
		size( std::exchange( donor.size, ZERO_INITIALIZATION ) ),
		msaa( std::exchange( donor.msaa, {} ) ),
		clear_targets( std::exchange( donor.clear_targets, {} ) ),
		clear_color( std::exchange( donor.clear_color, Color4::Black() ) ),
		clear_depth_value( std::exchange( donor.clear_depth_value, 0.0f ) ),
		clear_stencil_value( std::exchange( donor.clear_stencil_value, 0 ) ),
		name( std::exchange( donor.name, "<moved-from>" ) ),
		description( std::move( donor.description ) ),
		color_attachment( std::exchange( donor.color_attachment, nullptr ) ),
		depth_stencil_attachment( std::exchange( donor.depth_stencil_attachment, nullptr ) ),
		depth_attachment( std::exchange( donor.depth_attachment, nullptr ) ),
		stencil_attachment( std::exchange( donor.stencil_attachment, nullptr ) )
	{
	}

	Framebuffer& Framebuffer::operator=( Framebuffer&& donor )
	{
		Destroy();

		id                       = std::exchange( donor.id,							{} );
		bind_point               = std::exchange( donor.bind_point,					BindPoint::Invalid );
		size                     = std::exchange( donor.size,						ZERO_INITIALIZATION );
		msaa                     = std::exchange( donor.msaa,						{} );
		clear_targets            = std::exchange( donor.clear_targets,				{} );
		clear_color              = std::exchange( donor.clear_color,				Color4::Black() );
		clear_depth_value        = std::exchange( donor.clear_depth_value,			0.0f );
		clear_stencil_value      = std::exchange( donor.clear_stencil_value,		0 );
		name                     = std::exchange( donor.name,						"<moved-from>" );
		description              = std::move( donor.description );
		color_attachment         = std::exchange( donor.color_attachment,			nullptr );
		depth_stencil_attachment = std::exchange( donor.depth_stencil_attachment,	nullptr );
		depth_attachment         = std::exchange( donor.depth_attachment,			nullptr );
		stencil_attachment       = std::exchange( donor.stencil_attachment,			nullptr );

		return *this;
	}

	Framebuffer::~Framebuffer()
	{
		Destroy();
	}

	void Framebuffer::Resize( const int new_width_in_pixels, const int new_height_in_pixels )
	{
		Bind();

		description.width_in_pixels  = new_width_in_pixels;
		description.height_in_pixels = new_height_in_pixels;

		size = { new_width_in_pixels, new_height_in_pixels };

#ifdef _EDITOR
		if( not name.empty() )
		{
			const std::string full_name( IsMultiSampled()
										   ? this->name + " (MSAA " + std::to_string( msaa.sample_count ) + "x)"
										   : this->name );
			ServiceLocator< GLLogger >::Get().SetLabel( GL_FRAMEBUFFER, id.Get(), GL_LABEL_PREFIX_FRAMEBUFFER + full_name );
		}
#endif // _EDITOR

		CreateAttachments();

		Unbind();
	}

	void Framebuffer::SetName( const std::string& new_name )
	{
		name = new_name;
	}

	void Framebuffer::Blit( const Framebuffer& source, const Framebuffer& destination, const Texture::Filtering filtering )
	{
		ASSERT_EDITOR_ONLY( filtering == Texture::Filtering::Nearest || filtering == Texture::Filtering::Linear );

		glBindFramebuffer( GL_READ_FRAMEBUFFER, source.Id().Get() );
		glBindFramebuffer( GL_DRAW_FRAMEBUFFER, destination.Id().Get() );
		glBlitFramebuffer( 0, 0, source.Width(), source.Height(),
						   0, 0, destination.Width(), destination.Height(),
						   GL_COLOR_BUFFER_BIT, ( int )filtering );
	}

	void Framebuffer::Create()
	{
		glGenFramebuffers( 1, id.Address() );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
		{
			const std::string full_name( IsMultiSampled()
											? this->name + " (MSAA " + std::to_string( msaa.sample_count ) + "x)"
										    : this->name );
			ServiceLocator< GLLogger >::Get().SetLabel( GL_FRAMEBUFFER, id.Get(), GL_LABEL_PREFIX_FRAMEBUFFER + full_name );
		}
#endif // _EDITOR

		CreateAttachments();

		if( not HasColorAttachment() )
		{
			/* Disable color read/write since there's no color attachment attached. */
			glDrawBuffer( GL_NONE );
			glReadBuffer( GL_NONE );
		}

		ASSERT_DEBUG_ONLY( glCheckFramebufferStatus( GL_FRAMEBUFFER ) == GL_FRAMEBUFFER_COMPLETE );
		if( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
			throw std::runtime_error( "ERROR::FRAMEBUFFER::Framebuffer is not complete!" );

		Unbind();
	}

	void Framebuffer::CreateAttachments()
	{
		if( description.attachment_bits.IsSet( AttachmentType::Color ) )
		{
			CreateTextureAndAttachToFramebuffer( color_attachment, " Color ", GL_COLOR_ATTACHMENT0,
												 description.color_format == Texture::Format::NOT_ASSIGNED ? Texture::Format::RGBA : description.color_format,
												 description );
			clear_targets.Set( ClearTarget::ColorBuffer );
		}

		if( description.attachment_bits.IsSet( AttachmentType::DepthStencilCombined ) )
		{
			CreateTextureAndAttachToFramebuffer( depth_stencil_attachment, " D/S ", GL_DEPTH_STENCIL_ATTACHMENT, Texture::Format::DEPTH_STENCIL,
												 description );
			clear_targets.Set( ClearTarget::DepthBuffer, ClearTarget::StencilBuffer );
		}
		else
		{
			if( description.attachment_bits.IsSet( AttachmentType::Depth ) )
			{
				CreateTextureAndAttachToFramebuffer( depth_attachment, " Depth ", GL_DEPTH_ATTACHMENT, Texture::Format::DEPTH,
													 description );
				clear_targets.Set( ClearTarget::DepthBuffer );
			}
			if( description.attachment_bits.IsSet( AttachmentType::Stencil ) )
			{
				CreateTextureAndAttachToFramebuffer( stencil_attachment, " Stencil ", GL_STENCIL_ATTACHMENT, Texture::Format::STENCIL,
													 description );
				clear_targets.Set( ClearTarget::StencilBuffer );
			}
		}
	}

	void Framebuffer::CreateTextureAndAttachToFramebuffer( const Texture*& attachment_texture,
														   const char* attachment_type_name,
														   const GLenum attachment_type_enum,
														   const Texture::Format format,
														   const Description& description )
	{
		ASSERT_DEBUG_ONLY( ( ( attachment_type_enum >= GL_COLOR_ATTACHMENT0 && attachment_type_enum <= GL_DEPTH_ATTACHMENT ) ||
							 attachment_type_enum == GL_STENCIL_ATTACHMENT ||
							 attachment_type_enum == GL_DEPTH_STENCIL_ATTACHMENT ) &&
						   "Invalid attachment type enum passed to Texture::CreateTextureAndAttachToFramebuffer()" );

		if( IsMultiSampled() )
		{
			/* MSAA prefix/suffix here in the name is mandatory, 
			 * otherwise code below may update an already existing texture instead of creating a new one! */

			std::string full_name( this->name + attachment_type_name + std::to_string( size.X() ) + "x" + std::to_string( size.Y() ) +
								   " (MSAA " + std::to_string( msaa.sample_count ) + "x)" );
			attachment_texture = Engine::ServiceLocator< AssetDatabase< Engine::Texture > >::Get().AddOrUpdateAsset( Engine::Texture( full_name,
																																	  format,
																																	  msaa.sample_count,
																																	  size.X(),
																																	  size.Y() ) );
		}
		else
		{
			std::string full_name( this->name + attachment_type_name + std::to_string( size.X() ) + "x" + std::to_string( size.Y() ) );
			attachment_texture =
				Engine::ServiceLocator< AssetDatabase< Engine::Texture > >::Get().AddOrUpdateAsset( Engine::Texture( full_name, format,
																													 size.X(),
																													 size.Y(),
																													 description.wrap_u,
																													 description.wrap_v,
																													 description.border_color,
																													 description.minification_filter,
																													 description.magnification_filter ) );
		}

		glFramebufferTexture2D( ( GLenum )bind_point, attachment_type_enum, msaa.IsEnabled() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D, attachment_texture->Id().Get(), 0);
	}

	void Framebuffer::SetClearColor( const Color3& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

	void Framebuffer::SetClearColor( const Color4& new_clear_color )
	{
		clear_color = new_clear_color;
		SetClearColor();
	}

	void Framebuffer::SetClearDepthValue( const float new_clear_depth_value )
	{
		clear_depth_value = new_clear_depth_value;
		SetClearDepthValue();
	}

	void Framebuffer::SetClearStencilValue( const int new_clear_stencil_value )
	{
		clear_stencil_value = new_clear_stencil_value;
		SetClearStencilValue();
	}

	void Framebuffer::SetClearTargets( const BitFlags< ClearTarget > targets )
	{
		clear_targets = targets;
	}

	void Framebuffer::Clear() const
	{
		glClear( ( GLbitfield )clear_targets.ToBits() );
	}

	void Framebuffer::Destroy()
	{
		if( IsValid() ) // Also prevents the default framebuffer from getting processed here.
		{
			auto& texture_database = Engine::ServiceLocator< AssetDatabase< Engine::Texture > >::Get();

			if( HasColorAttachment() )
				texture_database.RemoveAsset( color_attachment->Name() );

			if( HasCombinedDepthStencilAttachment() )
				texture_database.RemoveAsset( depth_stencil_attachment->Name() );
			else
			{
				if( HasSeparateDepthAttachment() )
					texture_database.RemoveAsset( depth_attachment->Name() );
				if( HasSeparateStencilAttachment() )
					texture_database.RemoveAsset( stencil_attachment->Name() );
			}

			glDeleteFramebuffers( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void Framebuffer::Debug_FlashClearColor( bool& is_running, const Color4& start, const Color4& end, const float duration_in_seconds, const std::uint8_t ping_pong_count )
	{
		const auto current_clear_color = clear_color;

		ServiceLocator< MorphSystem >::Get().Add( Morph
		{
			.on_execute = [ this, start, end, ping_pong_count ]( float t )
			{
				const float single_lerp_duration = 1.0f / ( float )ping_pong_count;

				const std::uint8_t ping_pong_index = int( t / single_lerp_duration );

				if( ping_pong_index % 2 == 0 )
					clear_color = Math::Lerp( start, end, t * ping_pong_count - ping_pong_index );
				else
					clear_color = Math::Lerp( end, start, t * ping_pong_count - ping_pong_index );

				SetClearColor( clear_color );
			},
			.on_complete = [ this, current_clear_color, &is_running ]()
			{
				SetClearColor( clear_color = current_clear_color );
				is_running = false;
			},
			.duration_in_seconds = duration_in_seconds
		} );
	}

	void Framebuffer::SetClearColor()
	{
		glClearColor( clear_color.R(), clear_color.G(), clear_color.B(), clear_color.A() );
	}

	void Framebuffer::SetClearDepthValue()
	{
		glClearDepthf( clear_depth_value );
	}

	void Framebuffer::SetClearStencilValue()
	{
		glClearStencil( clear_stencil_value );
	}

	void Framebuffer::Bind() const
	{
		glBindFramebuffer( ( GLenum )bind_point, id.Get() );
	}

	void Framebuffer::Unbind() const
	{
		glBindFramebuffer( ( GLenum )bind_point, 0 );
	}
}
