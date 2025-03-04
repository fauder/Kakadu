/* Based on ImGui demo ExampleAppLog. */
#pragma once

// Engine Includes.
#include "Utility.hpp"

// Vendor Includes.
#include "ImGui/imgui.h"

// std Includes.
#include <array>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace Engine
{
	/* Only allow enums as the templated type. */
	template< typename Type, typename std::size_t Size, typename Enable = void >
	class ImGuiLog;

	template< typename Type, std::size_t Size >
	class ImGuiLog< Type, Size, typename std::enable_if_t< std::is_enum_v< Type > > >
	{
	public:
		ImGuiLog( const std::array< ImVec4, Size >& colors/*, const std::array< const char*, Size >& typeNames*/ )
			:
			autoScroll( true ),
			group( true ),
			colorsByType( colors )/*,
			typeNames( typeNames )*/
		{
			//std::fill( enabledTypes.begin(), enabledTypes.end(), true ); // Initially enable all log types.
			Clear();
		}

		void Clear()
		{
			allText.clear();
			lineTypes.clear();
			lineOffsets.clear();
			lineOffsets.push_back( 0 );
			line_counts_by_hash_map.clear();
		}

		void AddLog( const Type type, const char* text )
		{
			/* There may be multiple lines in text. */
			std::string_view text_view( text );
			const auto potential_lines = Utility::String::Split( text_view, '\n' );

			std::vector< std::string_view > lines;

			if( group )
			{
				for( const auto& line : potential_lines )
				{
					const std::size_t hash_of_line = std::hash< std::string_view >{}( line );
					if( auto iterator = line_counts_by_hash_map.find( hash_of_line );
						iterator != line_counts_by_hash_map.cend() )
					{
						iterator->second++;
					}
					else
					{
						lines.push_back( line );
						line_counts_by_hash_map.emplace( hash_of_line, 1 );
					}
				}
			}
			else
				lines = potential_lines;

			if( lines.empty() )
				return;

			for( const auto& line : lines )
				lineOffsets.push_back( lineOffsets.back() + ( int )line.size() + 1 ); // + 1 is for the new line character, which was stripped from each line in Split() above.

			allText.append( text );

			if( text_view.back() != '\n' )
				allText.append( "\n" ); // No need to increment the line offset of the last line here because the loop above always adds +1 to the offsets (for newline), even if there's no trailing newline yet.

			lineTypes.insert( lineTypes.begin() + lineTypes.size(), ( int )lines.size(), type );
		}

		void AddLog( const Type type, const std::string& text )
		{
			AddLog( type, text.c_str() );
		}

		void AddLogFormatted( const Type type, const char* fmt, ... ) IM_FMTARGS( 2 )
		{
			static char text[ 255 ];
			va_list args;
			va_start( args, fmt );
			vsprintf_s( text, fmt, args );
			va_end( args );

			AddLog( type, text );
		}

		void Draw( const char* title, bool* p_open = nullptr )
		{
			if( !ImGui::Begin( title, p_open ) )
			{
				ImGui::End();
				return;
			}

			/* Options popup menu. */
			if( ImGui::BeginPopup( "Options" ) )
			{
				ImGui::Checkbox( "Auto-Scroll", &autoScroll );
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
			filter.Draw( "Filters", -100.0f );

			// TODO: Add checkboxes and make enabling/disabling types of logs work. Need to work with clipper.
			///* 2nd row: Checkboxes. */
			//for( int i = 0; i < size; i++ )
			//{
			//	ImGui::PushStyleColor( ImGuiCol_Text, colorsByType.at( i ) );
			//	ImGui::Checkbox( typeNames[ i ], &enabledTypes[ i ] );
			//	ImGui::PopStyleColor();
			//	ImGui::SameLine();
			//}

			//ImGui::NewLine();
			ImGui::Separator();
			ImGui::BeginChild( "Scrolling", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar );

			if( clear )
				Clear();
			if( copy )
				ImGui::LogToClipboard();

			ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0, 0 ) );
			const char* buf = allText.begin();
			const char* buf_end = allText.end();
			if( filter.IsActive() )
			{
				// In this example we don't use the clipper when Filter is enabled.
				// This is because we don't have a random access on the result on our filter.
				// A real application processing logs with ten of thousands of entries may want to store the result of search/filter.
				// especially if the filtering function is not trivial (e.g. reg-exp).
				for( int line_no = 0; line_no < lineOffsets.size(); line_no++ )
				{
					const char* line_start = buf + lineOffsets[ line_no ];
					const char* line_end = ( line_no + 1 < lineOffsets.size() ) ? ( buf + lineOffsets[ line_no + 1 ] - 1 ) : buf_end;
					if( filter.PassFilter( line_start, line_end ) )
					{
						if( group )
						{
							std::string_view line_view( line_start, line_end );
							const std::size_t hash_of_line = std::hash< std::string_view >{}( line_view );
							const auto line_count = line_counts_by_hash_map.at( hash_of_line );
							ImGui::Text( line_count > 1 ? "(%7d) " : "          ", line_count );
							ImGui::SameLine();
						}
						ImGui::TextUnformatted( line_start, line_end );
					}
				}
			}
			else
			{
				// The simplest and easy way to display the entire buffer:
				//   ImGui::TextUnformatted(buf_begin, buf_end);
				// And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
				// to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
				// within the visible area.
				// If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
				// on your side is recommended. Using ImGuiListClipper requires
				// - A) random access into your data
				// - B) items all being the  same height,
				// both of which we can handle since we have an array pointing to the beginning of each line of text.
				// When using the filter (in the block of code above) we don't have random access into the data to display
				// anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
				// it possible (and would be recommended if you want to search through tens of thousands of entries).

				/*Clipper needs to know about total item count. So we need to find out how many lines are currently enabled. */
				ImGuiListClipper clipper;
				clipper.Begin( ( int )lineTypes.size() );
				while( clipper.Step() )
				{
					for( int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++ )
					{
						const char* line_start = buf + lineOffsets[ line_no ];
						const char* line_end   = ( line_no + 1 < lineOffsets.size() ) ? ( buf + lineOffsets[ line_no + 1 ] - 1 ) : buf_end;
						ImGui::PushStyleColor( ImGuiCol_Text, colorsByType.at( ( unsigned int )lineTypes[ line_no ] ) );

						if( group )
						{
							std::string_view line_view( line_start, line_end );
							const std::size_t hash_of_line = std::hash< std::string_view >{}( line_view );
							const auto line_count = line_counts_by_hash_map.at( hash_of_line );
							static char temp[ 25 ];
							snprintf( temp, 25, "(%d)", line_count );
							ImGui::Text( "%9s ", temp );
							ImGui::SameLine();
						}
						ImGui::TextUnformatted( line_start, line_end );
						ImGui::PopStyleColor();
					}

				}
				clipper.End();

				ImGui::TextUnformatted( "\n" );
			}
			ImGui::PopStyleVar();

			if( autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
				ImGui::SetScrollHereY( 1.0f );

			ImGui::EndChild();
			ImGui::End();
		}

		void SetColors( std::array< ImVec4, Size > newColorsByType )
		{
			colorsByType = newColorsByType;
		}

	private:
		ImGuiTextBuffer							allText;

		ImGuiTextFilter							filter;

		// Index to lines offset. We maintain this w/ AddLog() calls, allowing us random access on lines.
		std::vector< int >						lineOffsets; 

		bool									autoScroll;

		bool									group; // Group same logs under 1 line.
		//bool[ 6 ]								padding;
		std::unordered_map< std::size_t, int >	line_counts_by_hash_map;

		std::vector< Type >						lineTypes;
		std::array< ImVec4,						Size > colorsByType;
		//std::array< const char*,				Size > typeNames;
		//std::array< bool,						Size > enabledTypes;
	};
}
