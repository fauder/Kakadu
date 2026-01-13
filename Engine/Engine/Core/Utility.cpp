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
			std::string_view RemoveLeadingWhitespace( std::string_view source )
			{
				const auto pos = source.find_first_not_of( " \t" );
				if( pos != std::string_view::npos )
					source.remove_prefix( pos );
				else
					source.remove_prefix( source.size() ); // All whitespace.

				return source;
			}

			std::string_view RemoveTrailingWhitespace( std::string_view source )
			{
				const auto pos = source.find_last_not_of( " \t" );
				if( pos != std::string_view::npos )
					source.remove_suffix( source.size() - ( pos + 1 ) );
				else
					source.remove_suffix( source.size() ); // All whitespace.

				return source;
			}

			std::string_view RemoveWhitespace( const std::string_view source )
			{
				return RemoveLeadingWhitespace( RemoveTrailingWhitespace( source ) );
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

			std::vector< std::string_view > Split( std::string_view source, const std::string_view opening_delimiters, const std::string_view closing_delimiters )
			{
				std::vector< std::string_view > tokens;

				std::optional< std::string_view > maybe_token;
				while( maybe_token = ParseTokenAndAdvance( source, opening_delimiters, closing_delimiters ) )
					tokens.push_back( *maybe_token );

				if( !source.empty() && source.find_first_of( closing_delimiters ) == std::string::npos )
					tokens.push_back( source );

				return tokens;
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

			std::optional< std::string_view > ParseToken( const std::string_view source,
														  const std::string_view opening_delimiters,
														  const std::string_view closing_delimiters )
			{
				if( opening_delimiters.empty() )
				{
					if( const auto closing_delimiters_pos = source.find_first_of( closing_delimiters );
						closing_delimiters_pos != std::string_view::npos )
					{
						std::string_view parsed_token = source.substr( 0, closing_delimiters_pos );
						return parsed_token;
					}
				}
				else
				{
					if( const auto token_start_pos = source.find_first_not_of( opening_delimiters );
						token_start_pos != std::string_view::npos )
					{
						if( const auto token_end_pos = source.find_first_of( closing_delimiters, token_start_pos );
							token_end_pos != std::string_view::npos )
						{
							std::string_view parsed_token = source.substr( token_start_pos, token_end_pos - token_start_pos );
							return parsed_token;
						}
					}
				}

				return std::nullopt;
			}

			std::optional< std::string_view > ParseTokenAndAdvance( std::string_view& source_to_advance,
																	const std::string_view opening_delimiters,
																	const std::string_view closing_delimiters )
			{
				if( opening_delimiters.empty() )
				{
					if( const auto closing_delimiters_pos = source_to_advance.find_first_of( closing_delimiters );
						closing_delimiters_pos != std::string_view::npos )
					{
						std::string_view parsed_token = source_to_advance.substr( 0, closing_delimiters_pos );
						source_to_advance.remove_prefix( closing_delimiters_pos + 1 );
						return parsed_token;
					}
				}
				else
				{
					if( const auto token_start_pos = source_to_advance.find_first_not_of( opening_delimiters );
						token_start_pos != std::string_view::npos )
					{
						if( const auto token_end_pos = source_to_advance.find_first_of( closing_delimiters, token_start_pos );
							token_end_pos != std::string_view::npos )
						{
							std::string_view parsed_token = source_to_advance.substr( token_start_pos, token_end_pos - token_start_pos );
							source_to_advance.remove_prefix( token_end_pos + 1 );
							return parsed_token;
						}
					}
				}

				return std::nullopt;
			}

			std::optional< std::string_view > ParseTokenAndAdvance_SkipPrefix( std::string_view& source_to_advance,
																			   const std::string_view prefix_to_skip,
																			   const std::string_view opening_delimiters,
																			   const std::string_view closing_delimiters )
			{
				if( auto preceding_token_pos = source_to_advance.find( prefix_to_skip );
					preceding_token_pos == std::string_view::npos )
					return std::nullopt;
				else
				{
					source_to_advance.remove_prefix( preceding_token_pos + prefix_to_skip.size() );
					return ParseTokenAndAdvance( source_to_advance, opening_delimiters, closing_delimiters );
				}
			}

			std::optional< std::string_view > ParseTokenAndAdvance_SkipPrefixes( std::string_view& source_to_advance,
																				 std::initializer_list< const std::string_view > prefixes_to_skip,
																				 const std::string_view opening_delimiters,
																				 const std::string_view closing_delimiters )
			{
				std::size_t preceding_token_pos = 0;
				std::size_t last_preceding_token_size = 0;
				for( auto& preceding_token : prefixes_to_skip )
				{
					if( preceding_token_pos = source_to_advance.find( preceding_token, preceding_token_pos );
						preceding_token_pos == std::string_view::npos )
						return std::nullopt;

					last_preceding_token_size = preceding_token.size();
				}

				source_to_advance.remove_prefix( preceding_token_pos + last_preceding_token_size );

				return ParseTokenAndAdvance( source_to_advance, opening_delimiters, closing_delimiters );
			}

			std::vector< std::string_view > ParseAndSplitLine_SkipPrefix( std::string_view& source,
																		  const std::string_view prefix_to_skip,
																		  const std::string_view opening_delimiters,
																		  const std::string_view closing_delimiters )
			{
				if( auto preceding_token_pos = source.find( prefix_to_skip );
					preceding_token_pos != std::string_view::npos )
				{
					source.remove_prefix( preceding_token_pos + prefix_to_skip.size() );
					if( auto maybe_rest_of_the_line = ParseNextLineAndAdvance( source );
						maybe_rest_of_the_line.has_value() )
					{
						return Split( *maybe_rest_of_the_line, opening_delimiters, closing_delimiters );
					}
				}

				return {};
			}

			std::optional< std::string_view > ParseNextLine( std::string_view& source, const std::string_view end_delimiter )
			{
				return ParseToken( source, "", end_delimiter );
			}

			std::optional< std::string_view > ParseNextLineAndAdvance( std::string_view& source, const std::string_view end_delimiter )
			{
				return ParseTokenAndAdvance( source, "", end_delimiter );
			}

			std::string ToLowerAscii( const std::string_view s )
			{
				std::string out;
				out.reserve( s.size() );
				for( char c : s )
					out.push_back( static_cast< char >( std::tolower( static_cast< unsigned char >( c ) ) ) );
				return out;
			}

			void LowerAscii( std::string& s )
			{
				for( char& c : s )
					c = static_cast< char >( std::tolower( static_cast< unsigned char >( c ) ) );
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
