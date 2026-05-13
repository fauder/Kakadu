#pragma once

// Engine Includes.
#include "LogType.h"

// std Includes.
#include <string_view>

namespace Kakadu::Log::Sink
{
	using LogFunctionPointer = void( * )( Type, std::string_view );

	void Register( LogFunctionPointer log_func );
	void Unregister();

	void Dispatch( Type type, std::string_view message );
}
