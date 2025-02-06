#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Graphics.h"
#include "GraphicsMacros.h"
#include "ID.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <string>
#include <string_view>

namespace Engine
{
	template< typename AssetType >
	class AssetDatabase;

	enum class TextureType
	{
		None,

		Texture2D             = GL_TEXTURE_2D,
		Texture2D_MultiSample = GL_TEXTURE_2D_MULTISAMPLE,
		Cubemap               = GL_TEXTURE_CUBE_MAP
	};

	class Texture
	{
		struct CubeMapConstructorTag {};
		struct Texture2DMultiSampleConstructorTag {};

	public:
		static constexpr CubeMapConstructorTag CUBEMAP_CONSTRUCTOR = {};
		static constexpr Texture2DMultiSampleConstructorTag TEXTURE_2D_MULTISAMPLE_CONSTRUCTOR = {};

		enum class Format : std::uint8_t
		{
			NOT_ASSIGNED,

			R,
			RG,
			RGB,
			RGBA,

			RGBA_16F,
			RGBA_32F,

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
				case Format::SRGB:			return GAMMA_CORRECTION_IS_ENABLED ? Format::SRGB  : Format::RGB;
				case Format::SRGBA:			return GAMMA_CORRECTION_IS_ENABLED ? Format::SRGBA : Format::RGBA;
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
				case Format::SRGB:			return "[S]RGB";
				case Format::SRGBA:			return "[S]RGBA";
				case Format::DEPTH_STENCIL:	return "DEPTH_STENCIL";
				case Format::DEPTH:			return "DEPTH";
				case Format::STENCIL:		return "STENCIL";
				default:					return "UNKNOWN";
			}
		}

		enum class Wrapping // Has to be a default enum (4 bytes), because the GLenums are all over the place...
		{
			ClampToEdge       = GL_CLAMP_TO_EDGE,
			ClampToBorder     = GL_CLAMP_TO_BORDER,
			MirroredRepeat    = GL_MIRRORED_REPEAT,
			Repeat            = GL_REPEAT,
			MirrorClampToEdge = GL_MIRROR_CLAMP_TO_EDGE
		};

		enum class Filtering // Has to be a default enum (4 bytes), because the GLenums are all over the place...
		{
			Nearest				  = GL_NEAREST,
			Linear			      = GL_LINEAR,
			Nearest_MipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
			Linear_MipmapNearest  = GL_LINEAR_MIPMAP_NEAREST,
			Nearest_MipmapLinear  = GL_NEAREST_MIPMAP_LINEAR,
			Linear_MipmapLinear	  = GL_LINEAR_MIPMAP_LINEAR
		};

		struct ImportSettings
		{
			Wrapping wrap_u = Wrapping::ClampToEdge;
			Wrapping wrap_v = Wrapping::ClampToEdge;
			Wrapping wrap_w = Wrapping::ClampToEdge;

			Color4 border_color = Color4::Black();

			Filtering min_filter = Filtering::Linear_MipmapLinear;
			Filtering mag_filter = Filtering::Linear;

			bool flip_vertically  = true;
			bool generate_mipmaps = true;

			Format format = Format::SRGBA;
			
			/* 1 byte of padding. */
		};

		static constexpr ImportSettings DEFAULT_IMPORT_SETTINGS = {};

	private:
		friend class AssetDatabase< Texture >;
		
		ASSET_LOADER_CLASS_DECLARATION( Texture );

	public:
		using ID = ID< Texture >;

	public:
		Texture();
		/* Allocate-only constructor (no data). */
		Texture( const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const int width, const int height,
				 const Wrapping wrap_u = Wrapping::ClampToEdge, const Wrapping wrap_v = Wrapping::ClampToEdge,
				 const Color4 border_color = Color4::Black(),
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Multi-sampled allocate-only constructor (no data). */
		Texture( const int sample_count,
				 const std::string_view multi_sample_texture_name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const int width, const int height );

		/* Cubemap allocate-only constructor (no data). */
		Texture( CubeMapConstructorTag tag,
				 const std::string_view name,
				 //const std::byte* data, This is omitted from this public constructor.
				 const Format format,
				 const int width, const int height,
				 const Wrapping wrap_u = Wrapping::ClampToEdge, const Wrapping wrap_v = Wrapping::ClampToEdge, const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Color4 border_color = Color4::Black(),
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		DELETE_COPY_CONSTRUCTORS( Texture );

		/* Allow moving: */
		Texture( Texture&& );
		Texture& operator =( Texture&& );

		~Texture();

	/* Queries: */
		bool IsValid() const { return id.Get(); }

		inline ID					Id()				const { return id;													}
		inline const Vector2I&		Size()				const { return size;												}
		inline int					Width()				const { return size.X();											}
		inline int					Height()			const { return size.Y();											}
		inline TextureType			Type()				const { return type;												}
		inline const std::string&	Name()				const { return name;												}
		inline int					SampleCount()		const { return sample_count;										}
		inline bool					IsMultiSampled()	const { return sample_count;										}
		inline bool					Is_sRGB()			const { return format == Format::SRGB || format == Format::SRGBA;	}
		inline Format				PixelFormat()		const { return format;												}

	/* Usage: */
		void SetName( const std::string& new_name );
		void Activate( const int slot ) const;
		void GenerateMipmaps() const;

		static void ToggleGammaCorrection( const bool enable );

	private:
		/* Private regular constructor: Only the AssetDatabase< Texture > should be able to construct a Texture with data. */
		Texture( const std::string_view name,
				 const std::byte* data,
				 const Format format,
				 const int width, const int height,
				 const bool generate_mipmaps = true,
				 const Wrapping wrap_u = Wrapping::ClampToEdge, const Wrapping wrap_v = Wrapping::ClampToEdge,
				 const Color4 border_color = Color4::Black(),
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		/* Private cubemap constructor: Only the AssetDatabase< Texture > should be able to construct a cubemap Texture with data. */
		Texture( CubeMapConstructorTag tag, 
				 const std::string_view name,
				 const std::array< const std::byte*, 6 >& cubemap_data_array,
				 const Format format,
				 const int width, const int height,
				 const Wrapping wrap_u = Wrapping::ClampToEdge, const Wrapping wrap_v = Wrapping::ClampToEdge, const Wrapping wrap_w = Wrapping::ClampToEdge,
				 const Color4 border_color = Color4::Black(),
				 const Filtering min_filter = Filtering::Linear_MipmapLinear, const Filtering mag_filter = Filtering::Linear );

		void Delete();

	/* Usage: */
		void Bind() const;
		void Unbind() const;

		constexpr static int InternalFormat( const Texture::Format format );
		constexpr static GLenum PixelDataFormat( const Texture::Format format );
		constexpr static GLenum PixelDataType( const Texture::Format format );

	private:
		ID id;
		Vector2I size;
		TextureType type;
		std::string name;
		int sample_count;
		Format format;

		static bool GAMMA_CORRECTION_IS_ENABLED;

		/* 3 bytes of padding. */
	};
};
