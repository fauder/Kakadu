/* Based on ImGui demo ExampleAppLog.
 * Edited to suit the needs of the engine */
#pragma once

 // Engine Includes.
#include "Core/LogType.h"
#include "Core/Types.h"

// Vendor Includes.
#include "ImGui/imgui.h"

// std Includes.
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace Kakadu::Editor
{
	class LogPanel
	{
	private:
		struct UniqueEntryInfo
		{
			/* This has 1 less element count than the line count, because the first line has offset = 0, and therefore its offset is redundant. */
			std::vector< u16 > line_start_offsets;

			u16 unique_text_start; // This is filled when this unique entry is encountered for the first time and is added to the map.
			u16 unique_text_end;   // This is filled when this unique entry is encountered for the first time and is added to the map.

			u32 repeat_count = 1; // This is just used as a badge/number to display when grouping is on. Starts from 1.

			Log::Type type;

			u16 LineCount() const { return ( u16 )line_start_offsets.size() + 1; /* + 1 for the first line. */ };
		};

	public:
		LogPanel();

		void Clear();

		void AddLog( const Log::Type type, const char* text );
		void AddLog( const Log::Type type, const std::string& text );
		void AddLogFormatted( const Log::Type type, const char* fmt, ... );

		void Draw( const char* title, bool* p_open = nullptr );

	private:
		std::unordered_map< std::size_t, UniqueEntryInfo > unique_entry_map;
		std::vector< std::size_t > unique_entry_hashes_per_line;
		std::vector< std::size_t > unique_entry_hashes_per_unique_line;

		ImGuiTextBuffer unique_text_buffer; // Does not contain duplicate entries.

		std::array< ImVec4, ( std::size_t )Log::Type::COUNT > colors_by_type;

		u16 total_line_count;
		u16 total_unique_line_count;

		bool auto_scroll;
		bool group; // Group same logs under 1 line.
	};
}
