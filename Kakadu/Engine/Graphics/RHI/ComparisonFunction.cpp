// Engine Includes.
#include "RHI.h"
#include "ComparisonFunction.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 ComparisonFunctionToGLEnum( ComparisonFunction value )
	{
		switch( value )
		{
			case ComparisonFunction::Always:         return GL_ALWAYS;
			case ComparisonFunction::Never:          return GL_NEVER;
			case ComparisonFunction::Equal:          return GL_EQUAL;
			case ComparisonFunction::NotEqual:       return GL_NOTEQUAL;
			case ComparisonFunction::Less:           return GL_LESS;
			case ComparisonFunction::LessOrEqual:    return GL_LEQUAL;
			case ComparisonFunction::Greater:        return GL_GREATER;
			case ComparisonFunction::GreaterOrEqual: return GL_GEQUAL;
		}

		ASSERT( false && "Invalid value in Kakadu::RHI::ComparisonFunctionToGLEnum( value )!" );
		return GL_NONE;
	}
}