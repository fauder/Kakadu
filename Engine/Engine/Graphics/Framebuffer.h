#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Enums.h"
#include "MSAA.h"
#include "Texture.h"
#include "Core/BitFlags.hpp"

// std Includes.
#include <string>

namespace Engine
{
	/* Forward Declarations: */
	class Renderer;

	class Framebuffer
	{
		friend class Renderer;

	public:
		using ID = ID< Framebuffer >;

		enum class BindPoint
		{
			Invalid = 0,
			Both    = GL_FRAMEBUFFER,
			Draw    = GL_DRAW_FRAMEBUFFER,
			Read    = GL_READ_FRAMEBUFFER
		};

		enum class AttachmentType : std::uint8_t
		{
			Color                = 1,
			Depth                = 2,
			Stencil              = 4,
			DepthStencilCombined = 8,

			Color_Depth_Stencil        = Color | Depth | Stencil,
			Color_DepthStencilCombined = Color | DepthStencilCombined
		};

		struct Description
		{
			std::string name;

			int width_in_pixels;
			int height_in_pixels;

			Texture::Filtering minification_filter  = Texture::Filtering::Linear;
			Texture::Filtering magnification_filter = Texture::Filtering::Linear;
			Texture::Wrapping  wrap_u               = Texture::Wrapping::ClampToEdge;
			Texture::Wrapping  wrap_v               = Texture::Wrapping::ClampToEdge;
			Color4 border_color                     = Color4::Black();
			BindPoint bind_point                    = BindPoint::Both;

			Texture::Format color_format            = Texture::Format::RGBA;
			BitFlags< AttachmentType > attachment_bits;
			MSAA msaa; // No MSAA by default.
			// 1 byte of padding.
		};

	public:
		Framebuffer();
		Framebuffer( const Description& description );
		Framebuffer( Description&& description );

		DELETE_COPY_CONSTRUCTORS( Framebuffer );

		Framebuffer( Framebuffer&& );
		Framebuffer& operator =( Framebuffer&& );

		~Framebuffer();

	/* Usage: */
		void Resize( const int new_width_in_pixels, const int new_height_in_pixels );

	/* Queries: */
		bool IsValid() const { return id.IsValid(); } // Technically, this fails for the default framebuffer which has id 0, but it's not needed there anyway.

		inline const ID				Id()				const { return id;	 }
		
		inline const Vector2I&		Size()				const { return size; }
		inline int					Width()				const { return size.X(); }
		inline int					Height()			const { return size.Y(); }

		inline int					SampleCount()		const { return msaa.sample_count; }
		inline bool					IsMultiSampled()	const { return msaa.IsEnabled(); }

		/* Default framebuffer always uses sRGB Encoding. */
		inline bool					Is_sRGB()			const { return id.Get() == 0 || ( HasColorAttachment() && color_attachment->Is_sRGB() ); }
		inline bool					IsHDR()				const { return HasColorAttachment() && color_attachment->IsHDR(); }

		inline const std::string&	Name()				const { return name; }

	/* Attachment Queries: */

		inline bool HasColorAttachment()				const { return color_attachment; }
		inline bool HasSeparateDepthAttachment()		const { return depth_attachment && not stencil_attachment; }
		inline bool HasSeparateStencilAttachment()		const { return stencil_attachment && not depth_attachment; }
		inline bool HasCombinedDepthStencilAttachment()	const { return depth_stencil_attachment; }

		inline const Texture& ColorAttachment()			const { return *color_attachment; }
		inline const Texture& DepthStencilAttachment()	const { return *depth_stencil_attachment; }
		inline const Texture& DepthAttachment()			const { return *depth_attachment; }
		inline const Texture& StencilAttachment()		const { return *stencil_attachment; }

	private:
		struct DefaultFramebuferConstructorTag {};
		static constexpr DefaultFramebuferConstructorTag DEFAULT_FRAMEBUFFER_CONSTRUCTOR;

		Framebuffer( DefaultFramebuferConstructorTag );

	/* Usage: */
		void Bind() const;
		void Unbind() const;
		void SetName( const std::string& new_name );
		static void Blit( const Framebuffer& source, const Framebuffer& destination, const Texture::Filtering filtering = Texture::Filtering::Nearest );

		void Create();
		void CreateAttachments();
		void CreateTextureAndAttachToFramebuffer( const Texture*& attachment_texture,
												  const char* attachment_type_name,
												  const GLenum attachment_type_enum,
												  const Texture::Format format,
												  const Description& description );

	/* Clearing: */

		void SetClearColor( const Color3& new_clear_color );
		void SetClearColor( const Color4& new_clear_color );
		void SetClearDepthValue( const float new_clear_depth_value );
		void SetClearStencilValue( const int new_clear_stencil_value );

		void SetClearTargets( const BitFlags< ClearTarget > targets );
		void Clear() const;

		void Destroy();

#ifdef _EDITOR
		void Debug_FlashClearColor( bool& is_running,
									const Color4& start = Color4::Cyan(), const Color4& end = Color4::Yellow(),
									const float duration_in_seconds = 0.5f, const std::uint8_t ping_pong_count = 5 );
#endif // _EDITOR

	/* Clearing: */

		void SetClearColor();
		void SetClearDepthValue();
		void SetClearStencilValue();

	private:
		ID id;
		BindPoint bind_point;

		Vector2I size;

		MSAA msaa;
		// 3 bytes of padding.

		BitFlags< ClearTarget > clear_targets;
		Color4 clear_color;
		float clear_depth_value;
		int clear_stencil_value;

		std::string name;

		Description description;

		const Texture* color_attachment;
		const Texture* depth_stencil_attachment;
		const Texture* depth_attachment;
		const Texture* stencil_attachment;
	};
}
