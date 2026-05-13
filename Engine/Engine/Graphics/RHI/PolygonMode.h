#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct PolygonMode : u8
	{
		Point,
		Line,
		Fill,

		Wireframe = Line
	};

	u32 PolygonModeToGLEnum( PolygonMode );
}