// Engine Includes.
#include "RHI.h"
#include "Usage.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 UsageToGLEnum( Usage usage )
	{
		switch( usage )
		{
			case Usage::StaticDraw:  return GL_STATIC_DRAW;
			case Usage::DynamicDraw: return GL_DYNAMIC_DRAW;
		}

		ASSERT( false && "Invalid usage in Kakadu::RHI::UsageToGLEnum( usage )!" );
		return GL_NONE;
	}
}