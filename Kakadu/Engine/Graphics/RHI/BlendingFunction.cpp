// Engine Includes.
#include "RHI.h"
#include "BlendingFunction.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 BlendingFunctionToGLEnum( BlendingFunction blend_function )
	{
		switch( blend_function )
		{
			case BlendingFunction::Add:             return GL_FUNC_ADD;
			case BlendingFunction::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
			case BlendingFunction::Subtract:        return GL_FUNC_SUBTRACT;
			case BlendingFunction::Minimum:         return GL_MIN;
			case BlendingFunction::Maximum:         return GL_MAX;
		}

		ASSERT( false && "Invalid blend_function in Kakadu::RHI::BlendingFunctionToGLEnum( blend_function )!" );
		return GL_NONE;
	}
}