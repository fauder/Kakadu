#ifdef _WIN32
// Windows Includes.
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#endif // _WIN32


// Engine Includes.
#include "Utility.hpp"

// std Includes.
#include <fstream>
#include <iostream>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt )
		{
			std::ifstream file;
			file.exceptions( std::ifstream::failbit | std::ifstream::badbit );
			try
			{
				file.open( file_path );
				return std::string( ( std::istreambuf_iterator< char >( file ) ),
									( std::istreambuf_iterator< char >() ) );

			}
			catch( const std::ifstream::failure& e )
			{
				if( optional_error_prompt )
					std::cout << optional_error_prompt << "\n    " << e.what() << "\n";
				
				return std::nullopt;
			}
		}

		namespace String
		{
			std::string_view RemoveLeadingWhitespace( const std::string_view source )
			{
				return std::string_view( source.cbegin() + source.find_first_not_of( " \t" ), source.cend() );
			}

			std::string_view RemoveTrailingWhitespace( const std::string_view source )
			{
				if( source.back() == ' ' || source.back() == '\t' )
					return std::string_view( source.cbegin(), source.cbegin() + source.find_last_not_of( " \t" ) );

				return source; // No trailing whitespace found.
			}

			void Replace( std::string& source, const std::string_view find_this, const std::string_view replace_with_this )
			{
				size_t start_pos = 0;
				while( ( start_pos = source.find( find_this, start_pos ) ) != std::string::npos )
				{
					source.replace( start_pos, find_this.length(), replace_with_this );
					start_pos += replace_with_this.length(); // Handles the case where 'replace_with_this' is a substring of 'find_this'.
				}
			}

			/* Returns either the multiple splitted string views or the source string in case delimiter was never found. */
			std::vector< std::string_view > Split( std::string_view source, const char delimiter )
			{
				std::vector< std::string_view > splitted;

				int start = 0;
				for( int char_index = 0; char_index < source.size(); char_index++ )
				{
					if( source[ char_index ] == delimiter )
					{
						splitted.emplace_back( source.cbegin() + start, source.cbegin() + char_index );
						start = char_index + 1; // Does not matter if out-of-bounds; Loop will end before this OoB value can be used.
					}
				}

				if( splitted.empty() )
					splitted.push_back( source );
				else if( start < source.size() ) // Add the last word.
					splitted.emplace_back( source.cbegin() + start, source.cend() );

				return splitted;
			}

			std::string_view FindPreviousWord( const std::string_view source, const std::size_t offset )
			{
				const std::size_t last_char_pos = source.find_last_not_of( " \t", offset - 1 );

				if( last_char_pos != std::string::npos )
				{
					const std::size_t last_preceding_whitespace_pos = source.find_last_of( " \t", last_char_pos );

					if( last_preceding_whitespace_pos != std::string::npos )
					{
						return std::string_view( source.cbegin() + last_preceding_whitespace_pos + 1, source.begin() + last_char_pos + 1 );
					}

					// No whitespace found before the word.
					return std::string_view( source.cbegin(), source.begin() + last_char_pos + 1 );
				}

				return source; // Contains only white-space.
			}

			std::optional< std::string_view > ParseNextTokenAndAdvance( std::string_view& source, 
																		const std::string_view opening_delimiters, const std::string_view closing_delimiters )
			{
				if( opening_delimiters.empty() )
				{
					if( const auto closing_delimiters_pos = source.find_first_of( closing_delimiters );
						closing_delimiters_pos != std::string_view::npos )
					{
						std::string_view parsed_token = source.substr( 0, closing_delimiters_pos );
						source.remove_prefix( closing_delimiters_pos + 1 );
						return parsed_token;
					}
				}
				else
				{
					if( const auto opening_delimiters_pos = source.find_first_of( opening_delimiters );
						opening_delimiters_pos != std::string_view::npos )
					{
						if( const auto closing_delimiters_pos = source.find_first_of( closing_delimiters, opening_delimiters_pos + 1 );
							closing_delimiters_pos != std::string_view::npos )
						{
							std::string_view parsed_token = source.substr( opening_delimiters_pos + 1, closing_delimiters_pos - ( opening_delimiters_pos + 1 ) );
							source.remove_prefix( closing_delimiters_pos + 1 );
							return parsed_token;
						}
					}
				}

				return std::nullopt;
			}

			std::optional< std::string_view > ParseNextTokenAndAdvance_WithPrefix( std::string_view& source,
																				   const std::string_view preceding_token,
																				   const std::string_view opening_delimiters, const std::string_view closing_delimiters )
			{
				if( auto preceding_token_pos = source.find( preceding_token );
					preceding_token_pos == std::string_view::npos )
					return std::nullopt;
				else
				{
					source.remove_prefix( preceding_token_pos + 1 );
					return ParseNextTokenAndAdvance( source, opening_delimiters, closing_delimiters );
				}
			}

			std::optional< std::string_view > ParseNextTokenAndAdvance_WithPrefix( std::string_view& source,
																				   std::initializer_list< const std::string_view > preceding_tokens,
																				   const std::string_view opening_delimiters, const std::string_view closing_delimiters )
			{
				std::size_t preceding_token_pos = 0;
				for( auto& preceding_token : preceding_tokens )
					if( preceding_token_pos = source.find( preceding_token, preceding_token_pos );
						preceding_token_pos == std::string_view::npos )
						return std::nullopt;

				source.remove_prefix( preceding_token_pos + 1 );

				return ParseNextTokenAndAdvance( source, opening_delimiters, closing_delimiters );
			}

			std::optional< std::string_view > ParseNextLineAndAdvance( std::string_view& source )
			{
				return ParseNextTokenAndAdvance( source, "", "\n" );
			}

#ifdef _WIN32
			std::wstring ToWideString( const std::string& string )
			{
				if( string.empty() )
					return std::wstring();

				int size_needed = MultiByteToWideChar( CP_UTF8, 0, &string[ 0 ], ( int )string.size(), NULL, 0 );
				std::wstring result( size_needed, 0 );
				MultiByteToWideChar( CP_UTF8, 0, &string[ 0 ], ( int )string.size(), &result[ 0 ], size_needed );
				return result;
			}

			std::string ToNarrowString( const std::wstring& wstring )
			{
				if( wstring.empty() )
					return std::string();

				const int size_needed = WideCharToMultiByte( CP_UTF8, 0, &wstring[ 0 ], ( int )wstring.size(), NULL, 0, NULL, NULL );
				std::string result( size_needed, 0 );
				WideCharToMultiByte( CP_UTF8, 0, &wstring[ 0 ], ( int )wstring.size(), &result[ 0 ], size_needed, NULL, NULL );
				return result;
			}
#endif // _WIN32
		}
	}
}
