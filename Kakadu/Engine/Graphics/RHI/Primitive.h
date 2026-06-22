#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct Primitive : u8
	{
		Points,
		Lines,
		Line_loop,
		Line_strip,
		Triangles,
		Triangle_strip,
		Triangle_fan,
		Quads
	};

	u32 PrimitiveToGLEnum( Primitive );
}
