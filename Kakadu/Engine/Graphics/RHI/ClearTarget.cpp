// Engine Includes.
#include "RHI.h"
#include "ClearTarget.h"
#include "Core/BitFlags.hpp"

namespace Kakadu::RHI
{
	u32 ClearTargetFlagsToGLEnum( BitFlags< ClearTarget > flags )
	{
		u32 result = 0;

		if( flags.IsSet( ClearTarget::ColorBuffer ) )
			result |= GL_COLOR_BUFFER_BIT;

		if( flags.IsSet( ClearTarget::DepthBuffer ) )
			result |= GL_DEPTH_BUFFER_BIT;

		if( flags.IsSet( ClearTarget::StencilBuffer ) )
			result |= GL_STENCIL_BUFFER_BIT;

		return result;
	}
}