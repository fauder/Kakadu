#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct TextureType : u8
	{
		None,

		Texture2D,
		Texture2D_MultiSample,
		Cubemap 
	};

	u32 TextureTypeToGLEnum( TextureType );
}
