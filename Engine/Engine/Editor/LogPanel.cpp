// Engine Includes.
#include "LogPanel.h"
#include "Core/Utility.hpp"
#include "Math/Math.hpp"

// std Includes.
#include <string_view>

namespace Kakadu::Editor
{
	LogPanel::LogPanel()
		:
		colors_by_type(
			{
				/* Red for errors.				*/ ImVec4( 1.0f,  0.0f,  0.0f,  1.0f ), // <-- ERROR
				/* Yellow for warnings.			*/ ImVec4( 1.0f,  1.0f,  0.0f,  1.0f ), // <-- WARNING
				/* Green for success.			*/ ImVec4( 0.11f, 0.44f, 0.07f, 1.0f ), // <-- SUCCESS
				/* White for normal logs.		*/ ImVec4( 1.0f,  1.0f,  1.0f,  1.0f ), // <-- NORMAL
				/* Teal for group separators.	*/ ImVec4( 0.38f, 1.0f,  0.82f, 1.0f ), // <-- GROUP SEPARATOR
			} ),
		total_line_count( 0 ),
		total_unique_line_count( 0 ),
		auto_scroll( true ),
		group( true )
	{
	}

	void LogPanel::Clear()
	{
		unique_text_buffer.clear();
		unique_entry_map.clear();
		unique_entry_hashes_per_line.clear();
		unique_entry_hashes_per_unique_line.clear();

		total_line_count        = 0;
		total_unique_line_count = 0;
	}

	void LogPanel::AddLog( const Log::Type type, const char* text )
	{
		std::string_view text_view( text );

		const std::size_t entry_hash = std::hash< std::string_view >{}( text_view );
		decltype( unique_entry_map )::iterator iterator;
		if( iterator = unique_entry_map.find( entry_hash );
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
					unique_entry.line_start_offsets[ line_index - 1 ] =
						unique_entry.line_start_offsets[ line_index - 2 ] + ( u16 )lines[ line_index - 1 ].size() + 1; // + 1 for new-line.
				}
			}

			unique_text_buffer.append( text );

			if( text_view.back() != '\n' )
			{
				unique_text_buffer.append( "\n" );
				unique_entry.unique_text_end++;
			}

			iterator = unique_entry_map.insert( { entry_hash, unique_entry } ).first;

			total_unique_line_count += unique_entry.LineCount();

			unique_entry_hashes_per_unique_line.insert( unique_entry_hashes_per_unique_line.end(), unique_entry.LineCount(), entry_hash );
		}

		UniqueEntryInfo& unique_entry_definitely_existing_now = iterator->second;

		total_line_count += unique_entry_definitely_existing_now.LineCount();

		unique_entry_hashes_per_line.insert( unique_entry_hashes_per_line.end(), unique_entry_definitely_existing_now.LineCount(), entry_hash );
	}

	void LogPanel::AddLog( const Log::Type type, const std::string& text )
	{
		AddLog( type, text.c_str() );
	}

	void LogPanel::AddLogFormatted( const Log::Type type, const char* fmt, ... ) IM_FMTARGS( 2 )
	{
		char text[ 255 ];
		va_list args;
		va_start( args, fmt );
		vsprintf_s( text, fmt, args );
		va_end( args );

		AddLog( type, text );
	}

	void LogPanel::Draw( const char* title, bool* p_open )
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

		constexpr float badge_padding_x = 6.0f;
		constexpr float badge_padding_y = 6.0f;
		const	  float text_height     = ImGui::GetTextLineHeight();
		const	  float line_height     = text_height + badge_padding_y * 2.0f; // Make lines bigger to fit badge nicely.

		// |<----------------------------------------- content region width --------------------------------------->|
		// |                                                                                                        |
		// | Line text goes here and wraps at wrap_pos | badge_gap | badge_padding_x | badge_text | badge_padding_x |
		// |                                           ^           ^                                                ^
		// |                                        wrap_pos   badge start                                       line end

		// Wrap pos based on 4 digits max. Bigger than 4 digits => Will overlap with the log entry and that is fine.
		const float max_badge_text_width = ImGui::CalcTextSize( "9999" ).x;
		const float max_badge_width      = max_badge_text_width + badge_padding_x * 2.0f;
		const float badge_gap            = badge_padding_x;
		const auto  wrap_pos             = ImGui::GetContentRegionAvail().x - max_badge_width - badge_gap;

		ImGui::PushTextWrapPos( wrap_pos );

		// Using slighltly transparent colors so it is more legible when the badge is overllapping the log entries.
		const auto& badge_color      = ImGui::GetColorU32( ImGuiCol_ScrollbarGrab,	0.65f );
		const auto& badge_text_color = ImGui::GetColorU32( ImGuiCol_Text,			0.65f );

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

				auto LocalIndex = [ & ]() -> u16
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
				const char* unique_entry_end   = buffer_begin + ( i32 )unique_entry.unique_text_end;

				const char* line_start = unique_entry_start + ( i32 )( local_line_index ? unique_entry.line_start_offsets[ local_line_index - 1 ] : 0 );
				const char* line_end   = ( local_line_index + 1 ) < ( i32 )unique_entry.LineCount()
					? unique_entry_start + ( i32 )unique_entry.line_start_offsets[ local_line_index ]
					: unique_entry_end;

				ImGui::PushStyleColor( ImGuiCol_Text, colors_by_type.at( ( i32 )unique_entry.type ) );

				// TODO: Maybe move exclamation etc. icons here?

				ImGui::SetCursorPosY( ImGui::GetCursorPosY() + badge_padding_y );
				const auto cursor_pos_y = ImGui::GetCursorPosY();

				ImGui::TextWrapped( "%.*s", ( int )( line_end - line_start ), line_start );
				const auto cursor_pos_to_resume_from = ImGui::GetCursorPos();

				if( group && local_line_index == 0 )
				{
					/*
					 * Draw repeat count badge:
					 */

					char badge_text[ 16 ];
					snprintf( badge_text, 16, "%d", unique_entry.repeat_count );

					const float badge_text_width  = ImGui::CalcTextSize( badge_text ).x;
					const float badge_half_height = text_height * 0.5f + badge_padding_y;
					const float badge_half_width  = std::max( badge_half_height, badge_text_width * 0.5f + badge_padding_x );
					const float rounding          = badge_half_height; // Perfect circle for single digit case.

					const ImVec2 window_pos = ImGui::GetWindowPos();
					const ImVec2 scroll     = ImVec2( ImGui::GetScrollX(), ImGui::GetScrollY() );

					const float badge_right_edge = window_pos.x - scroll.x + ImGui::GetContentRegionAvail().x + ImGui::GetScrollX();
					const float center_x         = badge_right_edge - badge_half_width - badge_padding_x;
					const float center_y         = window_pos.y - scroll.y + cursor_pos_y + text_height * 0.5f + badge_padding_y * 0.5f;

					ImDrawList* draw_list = ImGui::GetWindowDrawList();

					draw_list->AddRectFilled( ImVec2( center_x - badge_half_width, center_y - badge_half_height ),
											  ImVec2( center_x + badge_half_width, center_y + badge_half_height ),
											  badge_color,
											  rounding );
					draw_list->AddText( ImVec2( center_x - badge_text_width * 0.5f, center_y - text_height * 0.5f ),
										badge_text_color,
										badge_text );

					/* Place a Dummy to reserve the badge's horizontal space in the layout so the horizontal scrollbar accounts for it,
					 * and also update the cursors properly, which the ImDrawList API does not.
					 *
					 * Vertical cursor advancement is handled separately after this block. */
					ImGui::PopTextWrapPos();
					ImGui::SetCursorPos( ImVec2( wrap_pos, cursor_pos_y ) );
					ImGui::Dummy( ImVec2( badge_half_width * 2.0f, line_height ) );
					ImGui::PushTextWrapPos( wrap_pos );
					ImGui::SetCursorPos( cursor_pos_to_resume_from );
				}

				ImGui::PopStyleColor();

				// Handle the vertical cursor positioning properly here:
				ImGui::SetCursorPosY( std::max( ImGui::GetCursorPosY() + badge_padding_y, cursor_pos_y + line_height ) );

				ImGui::Separator();
			}
		}
		clipper.End();

		ImGui::TextUnformatted( "\n" );

		ImGui::PopTextWrapPos();

		ImGui::PopStyleVar();

		if( auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
			ImGui::SetScrollHereY( 1.0f );

		ImGui::EndChild();
		ImGui::End();
	}
}
