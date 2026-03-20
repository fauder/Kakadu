// Engine Includes.
#include "RHI.h"
#include "Usage.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	i32 UsageToGLEnum( Usage usage )
	{
		switch( usage )
		{
			case Usage::StaticDraw:  return GL_STATIC_DRAW;
			case Usage::DynamicDraw: return GL_DYNAMIC_DRAW;
		}

		ASSERT( false && "Invalid usage in Kakadu::GL::UsageToGLEnum( usage 0 )!" );
		return GL_NONE;
	}
}