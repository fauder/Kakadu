// Engine Includes.
#include "RHI.h"
#include "DebugMessageType.h"

// std Includes.
#include <iostream>

namespace Kakadu::RHI
{
	DebugMessageType GLenumToGLLogType( u32 value )
	{
		switch( value )
		{
			case GL_DEBUG_TYPE_ERROR:				return DebugMessageType::ERROR_;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return DebugMessageType::WARNING;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	return DebugMessageType::WARNING;
			case GL_DEBUG_TYPE_PORTABILITY:			return DebugMessageType::WARNING;
			case GL_DEBUG_TYPE_PERFORMANCE:			return DebugMessageType::WARNING;

			case GL_DEBUG_TYPE_MARKER:				return DebugMessageType::MARKER;
			case GL_DEBUG_TYPE_PUSH_GROUP:			return DebugMessageType::PUSH_GROUP;
			case GL_DEBUG_TYPE_POP_GROUP:			return DebugMessageType::POP_GROUP;

			case GL_DEBUG_TYPE_OTHER:				return DebugMessageType::NORMAL;

			default:
				std::cerr << "Invalid GLenum passed to GLenumToGLLogType( GLenum value )!\n";
				return DebugMessageType::INVALID;
		}
	}
}