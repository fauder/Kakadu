// Engine Includes
#include "RHI.h"
#include "TextureType.h"
#include "Core/Optimization.h"

namespace Kakadu::RHI
{
	u32 TextureTypeToGLEnum( TextureType type )
	{
		switch( type )
		{
			case Kakadu::RHI::TextureType::Texture2D:				return GL_TEXTURE_2D;
			case Kakadu::RHI::TextureType::Texture2D_MultiSample:	return GL_TEXTURE_2D_MULTISAMPLE;
			case Kakadu::RHI::TextureType::Cubemap:					return GL_TEXTURE_CUBE_MAP;

			default:
				UNREACHABLE();
		}

		ASSERT( false && "Invalid usage in Kakadu::RHI::TextureTypeToGLEnum( type )!" );
		return 0;
	}
}