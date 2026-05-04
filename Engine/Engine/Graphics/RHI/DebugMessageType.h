#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	enum class DebugMessageType
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

	DebugMessageType GLenumToGLLogType( u32 value );
}
