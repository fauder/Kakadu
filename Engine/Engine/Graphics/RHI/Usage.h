#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct Usage : u8
	{
		StaticDraw,
		DynamicDraw
	};

	u32 UsageToGLEnum( Usage );
};
