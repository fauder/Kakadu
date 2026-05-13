#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct BlendingFactor : u8
	{
		Zero,
		One,
		SourceColor,
		OneMinusSourceColor,
		SourceAlpha,
		OneMinusSourceAlpha,
		DestinationAlpha,
		OneMinusDestinationAlpha,
		DestinationColor,
		OneMinusDestinationColor,
		SourceAlphaSaturate
	};

	u32 BlendingFactorToGLEnum( BlendingFactor );
}