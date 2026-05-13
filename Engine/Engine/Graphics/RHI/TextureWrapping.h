#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct TextureWrapping : u8
	{
		ClampToEdge,
		ClampToBorder,
		MirroredRepeat,
		Repeat,
		MirrorClampToEdge
	};

	u32 TextureWrappingToGLEnum( TextureWrapping );
};
