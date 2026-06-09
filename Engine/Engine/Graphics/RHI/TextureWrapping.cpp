// Engine Includes.
#include "RHI.h"
#include "TextureWrapping.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 TextureWrappingToGLEnum( TextureWrapping wrapping_mode )
	{
		switch( wrapping_mode )
		{
			case TextureWrapping::ClampToEdge       : return GL_CLAMP_TO_EDGE;
			case TextureWrapping::ClampToBorder     : return GL_CLAMP_TO_BORDER;
			case TextureWrapping::MirroredRepeat    : return GL_MIRRORED_REPEAT;
			case TextureWrapping::Repeat            : return GL_REPEAT;
			case TextureWrapping::MirrorClampToEdge : return GL_MIRROR_CLAMP_TO_EDGE;
		}

		ASSERT( false && "Invalid wrapping_mode in Kakadu::RHI::TextureWrappingToGLEnum( wrapping_mode )!" );
		return GL_NONE;
	}

	TextureWrapping TextureWrappingFromGLEnum( u32 gl_enum )
	{
		switch( gl_enum )
		{
			case GL_CLAMP_TO_EDGE        : return TextureWrapping::ClampToEdge;
			case GL_CLAMP_TO_BORDER      : return TextureWrapping::ClampToBorder;
			case GL_MIRRORED_REPEAT      : return TextureWrapping::MirroredRepeat;
			case GL_REPEAT               : return TextureWrapping::Repeat;
			case GL_MIRROR_CLAMP_TO_EDGE : return TextureWrapping::MirrorClampToEdge;
		}

		ASSERT( false && "Invalid gl_enum in Kakadu::RHI::TextureWrappingFromGLEnum( gl_enum )!" );
		return TextureWrapping::Repeat;
	}
}