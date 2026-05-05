// Engine Includes.
#include "ImGuiLogger.h"
#include "Utility.hpp"
#include "Math/Math.hpp"

// std Includes.
#include <string_view>

namespace Kakadu
{
	ImGuiLogger::ImGuiLogger()
		:
		colors_by_type( {
							/* Red for errors.				*/ ImVec4( 1.0f,  0.0f,  0.0f,  1.0f ), // <-- ERROR
							/* Yellow for warnings.			*/ ImVec4( 1.0f,  1.0f,  0.0f,  1.0f ), // <-- WARNING
							/* Green for success.			*/ ImVec4( 0.3f,  0.6f,  0.2f,  1.0f ), // <-- SUCCESS
							/* White for normal logs.		*/ ImVec4( 1.0f,  1.0f,  1.0f,  1.0f ), // <-- NORMAL
							/* Teal for group separators.	*/ ImVec4( 0.38f, 1.0f,  0.82f, 1.0f ), // <-- GROUP SEPARATOR
						} ),
		total_line_count( 0 ),
		total_unique_line_count( 0 ),
		auto_scroll( true ),
		group( true )
	{
	}


	void ImGuiLogger::Clear()
	{
		unique_text_buffer.clear();
		unique_entry_map.clear();
		unique_entry_hashes_per_line.clear();
		unique_entry_hashes_per_unique_line.clear();

		total_line_count        = 0;
		total_unique_line_count = 0;
	}

	void ImGuiLogger::AddLog( const EntryType type, const char* text )
	{
		std::string_view text_view( text );

		const std::size_t entry_hash = std::hash< std::string_view >{}( text_view );
		if( auto iterator = unique_entry_map.find( entry_hash );
			iterator != unique_entry_map.cend() )
		{
			iterator->second.repeat_count++;
		}
		else
		{
			const auto lines = Utility::String::Split( text_view, '\n' );

			UniqueEntryInfo unique_entry
			{
				.line_start_offsets = std::vector< u16 >( lines.size() - 1 ),

				.unique_text_start = ( u16 )unique_text_buffer.size(),
				.unique_text_end   = ( u16 )( unique_text_buffer.size() + text_view.size() ),

				.repeat_count = 1,

				.type = type
			};

			if( not unique_entry.line_start_offsets.empty() )
			{
				/* First line in group (index 0) has offset 0, naturally.
				 * Second line's (index 1) offset is calculated out-of-loop as it would need to reference the first line,
				 * which is not part of the offsets array. */

				unique_entry.line_start_offsets[ 0 ] = ( u16 )lines[ 0 ].size() + 1;
				// +1 is for the first line which is not represented in the line_start_offsets array.

				for( auto line_index = 2, accumulated_offset = 0; line_index < lines.size(); line_index++ )
				{
					unique_entry.line_start_offsets[ line_index - 1 ] = unique_entry.line_start_offsets[ line_index - 2 ] + ( u16 )lines[ line_index - 1 ].size() + 1; // + 1 for new-line.
				}
			}

			unique_text_buffer.append( text );

			if( text_view.back() != '\n' )
			{
				unique_text_buffer.append( "\n" );
				unique_entry.unique_text_end++;
			}

			unique_entry_map[ entry_hash ] = unique_entry;

			total_unique_line_count += unique_entry.LineCount();

			unique_entry_hashes_per_unique_line.insert( unique_entry_hashes_per_unique_line.end(), unique_entry.LineCount(), entry_hash );
		}

		UniqueEntryInfo& unique_entry_definitely_existing_now = unique_entry_map.find( entry_hash )->second;

		total_line_count += unique_entry_definitely_existing_now.LineCount();

		unique_entry_hashes_per_line.insert( unique_entry_hashes_per_line.end(), unique_entry_definitely_existing_now.LineCount(), entry_hash );
	}

	void ImGuiLogger::AddLog( const EntryType type, const std::string& text )
	{
		AddLog( type, text.c_str() );
	}

	void ImGuiLogger::AddLogFormatted( const EntryType type, const char* fmt, ... ) IM_FMTARGS( 2 )
	{
		char text[ 255 ];
		va_list args;
		va_start( args, fmt );
		vsprintf_s( text, fmt, args );
		va_end( args );

		AddLog( type, text );
	}

	void ImGuiLogger::Draw( const char* title, bool* p_open )
	{
		if( !ImGui::Begin( title, p_open ) )
		{
			ImGui::End();
			return;
		}

		/* Options popup menu. */
		if( ImGui::BeginPopup( "Options" ) )
		{
			ImGui::Checkbox( "Auto-Scroll", &auto_scroll );
			ImGui::Checkbox( "Group", &group );
			ImGui::EndPopup();
		}

		/* 1st row: Buttons. */
		if( ImGui::Button( "Options" ) )
			ImGui::OpenPopup( "Options" );
		ImGui::SameLine();
		bool clear = ImGui::Button( "Clear" );
		ImGui::SameLine();
		bool copy = ImGui::Button( "Copy" );
		ImGui::SameLine();

		ImGui::Separator();
		ImGui::BeginChild( "Scrolling", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );

		if( clear )
			Clear();
		if( copy )
			ImGui::LogToClipboard();

		ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );

		/* ImGui notes:
			* The simplest and easy way to display the entire buffer:
			*   ImGui::TextUnformatted(buf_begin, buf_end);
			* And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
			* to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
			* within the visible area.
			* If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
			* on your side is recommended. Using ImGuiListClipper requires
			* - A) random access into your data
			* - B) items all being the  same height,
			* both of which we can handle since we have an array pointing to the beginning of each line of text.
			* When using the filter (in the block of code above) we don't have random access into the data to display
			* anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
			* it possible (and would be recommended if you want to search through tens of thousands of entries).
			*
			* Clipper needs to know about total item count. So we need to find out how many lines are currently enabled. */
		ImGuiListClipper clipper;
		clipper.Begin( group ? total_unique_line_count : total_line_count );
		const char* buffer_begin        = unique_text_buffer.begin();
		const auto& correct_hash_lookup = group ? unique_entry_hashes_per_unique_line : unique_entry_hashes_per_line;
		while( clipper.Step() )
		{
			for( i32 line_index = clipper.DisplayStart; line_index < clipper.DisplayEnd; line_index++ )
			{
				const std::size_t unique_entry_hash = correct_hash_lookup[ line_index ];
				const UniqueEntryInfo& unique_entry = unique_entry_map[ unique_entry_hash ];

				auto LocalIndex = [ & ]()
				{
					if( unique_entry.LineCount() == 1 )
						return 0;

					i32 reverse_index = line_index - 1;
					for( ; reverse_index >= 0; reverse_index-- )
						if( correct_hash_lookup[ reverse_index ] != unique_entry_hash )
							break;

					return line_index == reverse_index + 1 ? 0 : line_index - reverse_index - 1;
				};

				const u16 local_line_index = LocalIndex();

				const char* unique_entry_start = buffer_begin + ( i32 )unique_entry.unique_text_start;
				const char* unique_entry_end = buffer_begin + ( i32 )unique_entry.unique_text_end;

				const char* line_start = unique_entry_start + ( i32 )( local_line_index ? unique_entry.line_start_offsets[ local_line_index - 1 ] : 0 );
				const char* line_end = ( local_line_index + 1 ) < ( i32 )unique_entry.LineCount()
					? unique_entry_start + ( i32 )unique_entry.line_start_offsets[ local_line_index ]
					: unique_entry_end;
				ImGui::PushStyleColor( ImGuiCol_Text, colors_by_type.at( ( i32 )unique_entry.type ) );

				if( group )
				{
					char temp[ 25 ];

					if( local_line_index == 0 )
						snprintf( temp, 25, "(%d)", unique_entry.repeat_count );
					else
						temp[ 0 ] = '\0';

					ImGui::Text( "%9s ", temp );
					ImGui::SameLine();
				}
				ImGui::TextUnformatted( line_start, line_end );
				ImGui::PopStyleColor();
			}
		}
		clipper.End();

		ImGui::TextUnformatted( "\n" );

		ImGui::PopStyleVar();

		if( auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
			ImGui::SetScrollHereY( 1.0f );

		ImGui::EndChild();
		ImGui::End();
	}
}
