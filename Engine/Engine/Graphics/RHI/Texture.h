#pragma once

// Engine Includes.
#include "MSAA.h"
#include "TextureFiltering.h"
#include "TextureType.h"
#include "TextureWrapping.h"
#include "Core/AssetLoader.Codegen.h"
#include "Core/Concepts.h"
#include "ID/TextureID.h"
#include "Math/Color.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <string>
#include <stdexcept>

namespace Kakadu
{
	template< Concepts::NotPointer AssetType >
	class AssetDatabase;
}

namespace Kakadu::RHI
{
	class Texture
	{
		struct CubeMapConstructorTag {};
		struct Texture2DMultiSampleConstructorTag {};

	public:
		static constexpr CubeMapConstructorTag CUBEMAP_CONSTRUCTOR = {};
		static constexpr Texture2DMultiSampleConstructorTag TEXTURE_2D_MULTISAMPLE_CONSTRUCTOR = {};

		enum class Format : u8
		{
			NOT_ASSIGNED,

			R,
			RG,
			RGB,
			RGBA,

			RGBA_16F,
			RGBA_32F,
			R11G11B10F,

			SRGB,
			SRGBA,

			DEPTH_STENCIL,
			DEPTH,
			STENCIL
		};

		static Format DetermineActualFormat( const Format format )
		{
			switch( format )
			{
				case Format::R:				return Format::R;
				case Format::RG:			return Format::RG;
				case Format::RGB:			return Format::RGB;
				case Format::RGBA:			return Format::RGBA;
				case Format::RGBA_16F:		return Format::RGBA_16F;
				case Format::RGBA_32F:		return Format::RGBA_32F;
				case Format::R11G11B10F:	return Format::R11G11B10F;
				case Format::SRGB:			return Format::SRGB;
				case Format::SRGBA:			return Format::SRGBA;
				case Format::DEPTH_STENCIL:	return Format::DEPTH_STENCIL;
				case Format::DEPTH:			return Format::DEPTH;
				case Format::STENCIL:		return Format::STENCIL;

				default:					return Format::NOT_ASSIGNED;
			}
		}

		static constexpr const char* FormatName( const Format format )
		{
			switch( format )
			{
				case Format::NOT_ASSIGNED:	return "NOT_ASSIGNED";
				case Format::R:				return "R";
				case Format::RG:			return "RG";
				case Format::RGB:			return "RGB";
				case Format::RGBA:			return "RGBA";
				case Format::RGBA_16F:		return "RGBA_16F";
				case Format::RGBA_32F:		return "RGBA_32F";
				case Format::R11G11B10F:	return "R11G11B10F";
				case Format::SRGB:			return "[S]RGB";
				case Format::SRGBA:			return "[S]RGBA";
				case Format::DEPTH_STENCIL:	return "DEPTH_STENCIL";
				case Format::DEPTH:			return "DEPTH";
				case Format::STENCIL:		return "STENCIL";
				default:					return "UNKNOWN";
			}
		}

		using SizeType = Vector2I;

		struct ImportSettings
		{
			TextureWrapping wrap_u = TextureWrapping::ClampToEdge;
			TextureWrapping wrap_v = TextureWrapping::ClampToEdge;

			Color4 border_color = Color4::Black();

			TextureFiltering min_filter = TextureFiltering::Linear_MipmapLinear;
			TextureFiltering mag_filter = TextureFiltering::Linear;

			bool flip_vertically  = true;
			bool generate_mipmaps = true;

			Format format = Format::SRGBA;
			
			MSAA msaa;
		};

		static constexpr ImportSettings DEFAULT_IMPORT_SETTINGS = {};

	private:
		friend class AssetDatabase< Texture >;
		
		ASSET_LOADER_CLASS_DECLARATION( Texture );

	public:
		Texture();
		/* Allocate-only constructor (no data). */
		Texture( const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const i32 width,
				 const i32 height,
				 const TextureWrapping wrap_u      = TextureWrapping::ClampToEdge,
				 const TextureWrapping wrap_v      = TextureWrapping::ClampToEdge,
				 const Color4 border_color         = Color4::Black(),
				 const TextureFiltering min_filter = TextureFiltering::Linear_MipmapLinear,
				 const TextureFiltering mag_filter = TextureFiltering::Linear );

		/* Multi-sampled allocate-only constructor (no data). */
		Texture( const std::string_view multi_sample_texture_name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const u8 sample_count,
				 const i32 width,
				 const i32 height );

		/* Cubemap allocate-only constructor (no data).
		 * No wrapping options for cubemaps as all three axes will default to clamp-to-edge, which is the only sensible option. */
		Texture( CubeMapConstructorTag tag,
				 const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const i32 width,
				 const i32 height,
				 const Color4 border_color         = Color4::Black(),
				 const TextureFiltering min_filter = TextureFiltering::Linear_MipmapLinear,
				 const TextureFiltering mag_filter = TextureFiltering::Linear );

		DELETE_COPY_CONSTRUCTORS( Texture );

		/* Allow moving: */
		Texture( Texture&& );
		Texture& operator =( Texture&& );

		~Texture();

	/* Queries: */
		bool IsValid() const { return ( bool )id; }

		RHI::TextureID		Id()						const { return id; }
		const Vector2I&		Size()						const { return size; }
		i32					Width()						const { return size.X(); }
		i32					Height()					const { return size.Y(); }
		TextureType			Type()						const { return type; }
		const std::string&	Name()						const { return name; }
		TextureWrapping		Wrapping_U()				const { return import_settings.wrap_u; }
		TextureWrapping		Wrapping_V()				const { return import_settings.wrap_v; }
		TextureFiltering	MinificationFiltering()		const { return import_settings.min_filter; }
		TextureFiltering	MagnificationFiltering()	const { return import_settings.mag_filter; }
		i32					SampleCount()				const { return import_settings.msaa.sample_count; }
		bool				IsMultiSampled()			const { return import_settings.msaa.IsEnabled(); }
		bool				Is_sRGB()					const { return import_settings.format == Format::SRGB || import_settings.format == Format::SRGBA; }
		bool				IsHDR()						const { return
																	import_settings.format == Format::RGBA_16F ||
																	import_settings.format == Format::RGBA_32F ||
																	import_settings.format == Format::R11G11B10F; }
		Format				PixelFormat()				const { return import_settings.format; }

	/* Usage: */
		void SetName( const std::string& new_name );
		void Activate( const i32 slot ) const;
		void GenerateMipmaps() const;

		static i32 InternalFormat( const Texture::Format format );
		static u32 PixelDataFormat( const Texture::Format format );
		static u32 PixelDataType( const Texture::Format format );

	private:
		/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data. */
		Texture( const std::string_view name,
				 const std::byte* data,
				 const Format format,
				 const i32 width,
				 const i32 height,
				 const bool generate_mipmaps       = true,
				 const TextureWrapping wrap_u      = TextureWrapping::ClampToEdge,
				 const TextureWrapping wrap_v      = TextureWrapping::ClampToEdge,
				 const Color4 border_color         = Color4::Black(),
				 const TextureFiltering min_filter = TextureFiltering::Linear_MipmapLinear,
				 const TextureFiltering mag_filter = TextureFiltering::Linear );

		/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data.
		 * No wrapping options for cubemaps as all three axes will default to clamp-to-edge, which is the only sensible option. */
		Texture( CubeMapConstructorTag tag,
				 const std::string_view name,
				 const std::array< const std::byte*, 6 >& cubemap_data_array,
				 const Format format,
				 const i32 width,
				 const i32 height,
				 const Color4 border_color         = Color4::Black(),
				 const TextureFiltering min_filter = TextureFiltering::Linear_MipmapLinear,
				 const TextureFiltering mag_filter = TextureFiltering::Linear );

		void Delete();

	/* Usage: */
		void Bind() const;
		void Unbind() const;

	private:
		RHI::TextureID id;
		Vector2I size;
		TextureType type;
		std::string name;

		ImportSettings import_settings;

		/* 4 bytes of padding. */
	};
};
