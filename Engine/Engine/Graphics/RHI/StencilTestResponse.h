#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct StencilTestResponse : u8
	{
		Keep,
		Zero,
		Replace,
		Increment,
		IncrementWrap,
		Decrement,
		DecrementWrap,
		Invert
	};

	u32 StencilTestResponseToGLEnum( StencilTestResponse );
}