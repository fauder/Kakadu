#pragma once

// Engine Includes.
#include "Core/Concepts.h"
#include "Core/Types.h"

/* Forward Declarations: */
namespace Kakadu
{
	template< Concepts::IsEnum EnumType >
	class BitFlags;
}

namespace Kakadu::RHI
{
	enum class ClearTarget : u32
	{
		None = 0,

		DepthBuffer   = 0x1,
		StencilBuffer = 0x2,
		ColorBuffer   = 0x4,

		All = DepthBuffer | StencilBuffer | ColorBuffer
	};

	u32 ClearTargetFlagsToGLEnum( BitFlags< ClearTarget > flags );
}
