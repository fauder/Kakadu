#pragma once

#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct WindingOrder : u8
	{
		Clockwise,
		CounterClockwise
	};

	u32 WindingOrderToGLEnum( WindingOrder );
}