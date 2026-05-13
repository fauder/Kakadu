#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct ComparisonFunction : u8
	{
		Always,
		Never,
		Equal,
		NotEqual,
		Less,
		LessOrEqual,
		Greater,
		GreaterOrEqual
	};

	u32 ComparisonFunctionToGLEnum( ComparisonFunction );
}