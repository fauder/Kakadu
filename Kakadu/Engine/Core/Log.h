#pragma once

// std Includes.
#include <string_view>

namespace Kakadu::Log
{
	void Info( std::string_view message );
	void Warning( std::string_view warning_message );
	void Error( std::string_view error_message );
	void Success( std::string_view success_message );
};
