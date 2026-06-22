// Engine Includes.
#include "LogSink.h"
#include "Macros.h"

namespace Kakadu::Log::Sink
{
	internal_variable LogFunctionPointer LOG_FUNCTION_POINTER = nullptr;

	void Register( LogFunctionPointer log_func )
	{
		LOG_FUNCTION_POINTER = log_func;
	}

	void Unregister()
	{
		LOG_FUNCTION_POINTER = nullptr;
	}

	void Dispatch( Type type, std::string_view message )
	{
		if( LOG_FUNCTION_POINTER )
			LOG_FUNCTION_POINTER( type, message );
	}
}
