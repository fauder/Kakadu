// Engine Includes.
#include "RHI.h"
#include "StencilTestResponse.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 StencilTestResponseToGLEnum( StencilTestResponse operation )
	{
		switch( operation )
		{
			case StencilTestResponse::Keep:          return GL_KEEP;
			case StencilTestResponse::Zero:          return GL_ZERO;
			case StencilTestResponse::Replace:       return GL_REPLACE;
			case StencilTestResponse::Increment:     return GL_INCR;
			case StencilTestResponse::IncrementWrap: return GL_INCR_WRAP;
			case StencilTestResponse::Decrement:     return GL_DECR;
			case StencilTestResponse::DecrementWrap: return GL_DECR_WRAP;
			case StencilTestResponse::Invert:        return GL_INVERT;
		}

		ASSERT( false && "Invalid operation in Kakadu::RHI::StencilTestResponseToGLEnum( operation )!" );
		return GL_NONE;
	}
}