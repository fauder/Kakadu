#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Core/Macros.h"

// std Includes.
#include <iostream>

namespace Engine
{
	enum class GLLogType
	{
		ERROR_, // wingdi.h has ERROR macro...
		WARNING,
		MARKER,
		PUSH_GROUP,
		POP_GROUP,
		NORMAL,          

		COUNT,

		INVALID
	};

	header_function GLLogType GLenumToGLLogType( GLenum value )
	{
		switch( value )
		{
			case GL_DEBUG_TYPE_ERROR               : return GLLogType::ERROR_;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR : return GLLogType::WARNING;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR  : return GLLogType::WARNING;
			case GL_DEBUG_TYPE_PORTABILITY         : return GLLogType::WARNING;
			case GL_DEBUG_TYPE_PERFORMANCE         : return GLLogType::WARNING;

			case GL_DEBUG_TYPE_MARKER              : return GLLogType::MARKER;
			case GL_DEBUG_TYPE_PUSH_GROUP          : return GLLogType::PUSH_GROUP;
			case GL_DEBUG_TYPE_POP_GROUP           : return GLLogType::POP_GROUP;

			case GL_DEBUG_TYPE_OTHER               : return GLLogType::NORMAL;

			default:
				std::cerr << "Invalid GLenum passed to GLenumToGLLogType( GLenum value )!\n";
				return GLLogType::INVALID;
		}
	}
}
