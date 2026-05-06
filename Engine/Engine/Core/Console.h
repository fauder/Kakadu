#pragma once

// Engine Includes.
#include "ImGuiLogger.h"

// std Includes.
#include <string_view>

namespace Kakadu
{
	class Console
	{
	public:
		Console( ImGuiLogger& imgui_logger );

		void Log( std::string_view message );
		void LogWarning( std::string_view warning_message );
		void LogError( std::string_view error_message );
		void LogSuccess( std::string_view success_message );

		void Log( ImGuiLogger::EntryType type, std::string_view message );

	private:
		ImGuiLogger& imgui_logger;
	};
}
