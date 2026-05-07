// Engine Includes
#include "Console.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Kakadu
{
	Console::Console( ImGuiLogger& imgui_logger )
		:
		imgui_logger( imgui_logger )
	{}

	void Console::Log( std::string_view message )
	{
		Log( ImGuiLogger::EntryType::NORMAL, message );
	}

	void Console::LogWarning( std::string_view warning_message )
	{
		Log( ImGuiLogger::EntryType::WARNING, warning_message );
	}

	void Console::LogError( std::string_view error_message )
	{
		Log( ImGuiLogger::EntryType::ERROR_, error_message );
	}

	void Console::LogSuccess( std::string_view success_message )
	{
		Log( ImGuiLogger::EntryType::SUCCESS, success_message );
	}

	void Console::Log( ImGuiLogger::EntryType type, std::string_view message )
	{
		switch( type )
		{
			case ImGuiLogger::EntryType::ERROR_:
				imgui_logger.AddLogFormatted( ImGuiLogger::EntryType::ERROR_, "%s %s", ICON_FA_CIRCLE_EXCLAMATION, message.data() );
				break;
			case ImGuiLogger::EntryType::WARNING:
				imgui_logger.AddLogFormatted( ImGuiLogger::EntryType::WARNING, "%s %s", ICON_FA_TRIANGLE_EXCLAMATION, message.data() );
				break;
			case ImGuiLogger::EntryType::SUCCESS:
				imgui_logger.AddLogFormatted( ImGuiLogger::EntryType::SUCCESS, "%s %s", ICON_FA_CIRCLE_CHECK, message.data() );
				break;
			case ImGuiLogger::EntryType::NORMAL:
				imgui_logger.AddLog( ImGuiLogger::EntryType::NORMAL, message.data() );
				break;
			case ImGuiLogger::EntryType::GROUP_SEPARATOR:
				imgui_logger.AddLog( ImGuiLogger::EntryType::GROUP_SEPARATOR, message.data() );
				break;
			default:
				break;
		}
	}
}