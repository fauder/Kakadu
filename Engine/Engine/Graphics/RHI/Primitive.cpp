// Engine Includes. 
#include "RHI.h"
#include "Primitive.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 PrimitiveToGLEnum( Primitive primitive )
	{
		switch( primitive )
		{
			case Kakadu::RHI::Primitive::Points:			return GL_POINTS;
			case Kakadu::RHI::Primitive::Lines:				return GL_LINES;
			case Kakadu::RHI::Primitive::Line_loop:			return GL_LINE_LOOP;
			case Kakadu::RHI::Primitive::Line_strip:		return GL_LINE_STRIP;
			case Kakadu::RHI::Primitive::Triangles:			return GL_TRIANGLES;
			case Kakadu::RHI::Primitive::Triangle_strip:	return GL_TRIANGLE_STRIP;
			case Kakadu::RHI::Primitive::Triangle_fan:		return GL_TRIANGLE_FAN;
			case Kakadu::RHI::Primitive::Quads:				return GL_QUADS;
		}

		ASSERT( false && "Invalid primitive in Kakadu::RHI::PrimitiveToGLEnum( primitive )!" );
		return GL_NONE;
	}
}
