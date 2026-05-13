#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum struct Face : u8
	{
		Front,
		Back,
		FrontAndBack
	};

	u32 FaceToGLEnum( Face );
}