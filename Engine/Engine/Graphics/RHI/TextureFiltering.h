#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct TextureFiltering : u8
	{
		Nearest,
		Linear,
		Nearest_MipmapNearest,
		Linear_MipmapNearest,
		Nearest_MipmapLinear,
		Linear_MipmapLinear
	};

	u32 TextureFilteringToGLEnum( TextureFiltering );
};
