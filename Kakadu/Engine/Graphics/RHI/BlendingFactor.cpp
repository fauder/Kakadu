// Engine Includes.
#include "RHI.h"
#include "BlendingFactor.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 BlendingFactorToGLEnum( BlendingFactor factor )
	{
		switch( factor )
		{
			case BlendingFactor::Zero:                     return GL_ZERO;
			case BlendingFactor::One:                      return GL_ONE;
			case BlendingFactor::SourceColor:              return GL_SRC_COLOR;
			case BlendingFactor::OneMinusSourceColor:      return GL_ONE_MINUS_SRC_COLOR;
			case BlendingFactor::SourceAlpha:              return GL_SRC_ALPHA;
			case BlendingFactor::OneMinusSourceAlpha:      return GL_ONE_MINUS_SRC_ALPHA;
			case BlendingFactor::DestinationAlpha:         return GL_DST_ALPHA;
			case BlendingFactor::OneMinusDestinationAlpha: return GL_ONE_MINUS_DST_ALPHA;
			case BlendingFactor::DestinationColor:         return GL_DST_COLOR;
			case BlendingFactor::OneMinusDestinationColor: return GL_ONE_MINUS_DST_COLOR;
			case BlendingFactor::SourceAlphaSaturate:      return GL_SRC_ALPHA_SATURATE;
		}

		ASSERT( false && "Invalid factor in Kakadu::RHI::BlendingFactorToGLEnum( factor )!" );
		return GL_NONE;
	}
}