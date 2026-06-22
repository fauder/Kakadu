// Engine Includes.
#include "RHI.h"
#include "Face.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 FaceToGLEnum( Face face )
	{
		switch( face )
		{
			case Face::Front:        return GL_FRONT;
			case Face::Back:         return GL_BACK;
			case Face::FrontAndBack: return GL_FRONT_AND_BACK;
		}

		ASSERT( false && "Invalid face in Kakadu::RHI::FaceToGLEnum( face )!" );
		return GL_NONE;
	}
}