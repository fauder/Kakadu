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

	i32 UsageToGLEnum( Usage );
};
