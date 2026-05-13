// Engine Includes.
#include "RHI.h"
#include "PolygonMode.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 PolygonModeToGLEnum( PolygonMode mode )
	{
		switch( mode )
		{
			case PolygonMode::Point: return GL_POINT;
			case PolygonMode::Line:  return GL_LINE;
			case PolygonMode::Fill:  return GL_FILL;
		}

		ASSERT( false && "Invalid mode in Kakadu::RHI::PolygonModeToGLEnum( mode )!" );
		return GL_NONE;
	}
}