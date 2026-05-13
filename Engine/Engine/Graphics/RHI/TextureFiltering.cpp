// Engine Includes.
#include "RHI.h"
#include "TextureFiltering.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 TextureFilteringToGLEnum( TextureFiltering filtering_mode )
	{
		switch( filtering_mode )
		{
			case TextureFiltering::Nearest               : return GL_NEAREST;
			case TextureFiltering::Linear                : return GL_LINEAR;
			case TextureFiltering::Nearest_MipmapNearest : return GL_NEAREST_MIPMAP_NEAREST;
			case TextureFiltering::Linear_MipmapNearest  : return GL_LINEAR_MIPMAP_NEAREST;
			case TextureFiltering::Nearest_MipmapLinear  : return GL_NEAREST_MIPMAP_LINEAR;
			case TextureFiltering::Linear_MipmapLinear   : return GL_LINEAR_MIPMAP_LINEAR;
		}

		ASSERT( false && "Invalid filtering_mode in Kakadu::RHI::TextureFilteringToGLEnum( filtering_mode )!" );
		return GL_NONE;
	}
}