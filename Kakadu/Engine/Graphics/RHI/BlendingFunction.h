#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct BlendingFunction : u8
	{
		Add,
		ReverseSubtract,
		Subtract,
		Minimum,
		Maximum
	};

	u32 BlendingFunctionToGLEnum( BlendingFunction );
}