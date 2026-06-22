// Engine Includes.
#include "Log.h"
#include "LogSink.h"

namespace Kakadu::Log
{
	void Info( std::string_view message )
	{
		Sink::Dispatch( Type::NORMAL, message );
	}

	void Warning( std::string_view warning_message )
	{
		Sink::Dispatch( Type::WARNING, warning_message );
	}

	void Error( std::string_view error_message )
	{
		Sink::Dispatch( Type::ERROR_, error_message );
	}

	void Success( std::string_view success_message )
	{
		Sink::Dispatch( Type::SUCCESS, success_message );
	}
}
