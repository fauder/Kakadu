#pragma once

// Engine Includes.
#include "Assertion.h"

// std Includes.
#include <array>
#include <charconv>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace Engine
{
	namespace Utility
	{
		std::optional< std::string > ReadFileIntoString( const char* file_path, const char* optional_error_prompt = nullptr );

		namespace String
		{
			std::string_view RemoveLeadingWhitespace( const std::string_view source );
			std::string_view RemoveTrailingWhitespace( const std::string_view source );
			std::string_view RemoveWhitespace( const std::string_view source );
			std::string_view FindPreviousWord( const std::string_view source, const std::size_t offset );

			std::optional< std::string_view > ParseToken( const std::string_view source,
														  const std::string_view opening_delimiters = " \t",
														  const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseTokenAndAdvance( std::string_view& source_to_advance,
																	const std::string_view opening_delimiters = " \t",
																	const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseTokenAndAdvance_SkipPrefix( std::string_view& source_to_advance,
																			   const std::string_view prefix_to_skip,
																			   const std::string_view opening_delimiters = " \t",
																			   const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseTokenAndAdvance_SkipPrefix( std::string_view& source_to_advance,
																			   std::initializer_list< const std::string_view > prefixes_to_skip,
																			   const std::string_view opening_delimiters = " \t",
																			   const std::string_view closing_delimiters = " \t" );
			std::vector< std::string_view > ParseAndSplitLine_SkipPrefix( std::string_view& source,
																		  const std::string_view prefix_to_skip,
																		  const std::string_view opening_delimiters = " \t",
																		  const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseNextLine( std::string_view& source, const std::string_view end_delimiter = "\n" );
			std::optional< std::string_view > ParseNextLineAndAdvance( std::string_view& source, const std::string_view end_delimiter = "\n" );

			void Replace( std::string& source, const std::string_view find_this, const std::string_view replace_with_this );

			/* Returns either the multiple splitted string views or the source string in case delimiter was never found. */
			std::vector< std::string_view > Split( std::string_view source, const char delimiter );

			std::vector< std::string_view > Split( std::string_view source,
												   const std::string_view opening_delimiters = " \t",
												   const std::string_view closing_delimiters = " \t" );

			/* https://stackoverflow.com/a/75619411/4495751 */
			template< unsigned ... Length >
			constexpr auto ConstexprConcatenate( const char( &...strings )[ Length ] )
			{
				constexpr unsigned count = ( ... + Length ) - sizeof...( Length );
				std::array< char, count + 1 > result = {};
				result[ count ] = '\0';

				auto it = result.begin();
				( void )( ( it = std::copy_n( strings, Length - 1, it ), 0 ), ... );
				return result;
			}

			template< size_t Array1_Size, size_t Array2_Size >
			constexpr std::array< char, Array1_Size + Array2_Size + 1 > ConstexprConcatenate( const std::array< char, Array1_Size >& string_1,
																							  const std::array< char, Array2_Size >& string_2 )
			{
				std::array< char, Array1_Size + Array2_Size + 1 > result = {};

				// Copy contents of string_1:
				for( size_t i = 0; i < Array1_Size; ++i )
					result[ i ] = string_1[ i ];

				// Copy contents of string_2:
				for( size_t i = 0; i < Array2_Size; ++i )
					result[ Array1_Size + i ] = string_2[ i ];

				result[ Array1_Size + Array2_Size ] = '\0';

				return result;
			}

			// Helper constexpr function to create std::array from std::string_view.
			template< size_t Size >
			constexpr std::array< char, Size > StringViewToArray( std::string_view view )
			{
				std::array< char, Size > result = {};
				for( size_t i = 0; i < Size; ++i )
					result[ i ] = view[ i ];

				return result;
			}

			template< std::integral Type >
			Type ConvertToNumber( const std::string_view as_string )
			{
				Type numeric_value{ 0 };
#ifdef _EDITOR
				auto [ ptr, ec ] = std::from_chars( as_string.data(), as_string.data() + as_string.size(), numeric_value );
				if( ec == std::errc::invalid_argument )
					std::cerr << "Engine::Utility::String::ConvertToNumber< " << typeid( Type ).name() << " >(): String \"" << as_string
							  << "\" can not be converted to a number.\n";
#else
				std::from_chars( as_string.data(), as_string.data() + as_string.size(), numeric_value );
#endif // _EDITOR
				return numeric_value;
			}

			template< std::floating_point Type >
			Type ConvertToNumber( const std::string_view as_string, const std::chars_format format = std::chars_format::general )
			{
				Type numeric_value{ 0 };
#ifdef _EDITOR
				auto [ ptr, ec ] = std::from_chars( as_string.data(), as_string.data() + as_string.size(), numeric_value, format );
				if( ec == std::errc::invalid_argument )
					std::cerr << "Engine::Utility::String::ConvertToNumber< " << typeid( Type ).name() << " >(): String \"" << as_string
							  << "\" can not be converted to a number.\n";
#else
				std::from_chars( as_string.data(), as_string.data() + as_string.size(), numeric_value );
#endif // _EDITOR
				return numeric_value;
			}

#ifdef _WIN32
			std::wstring ToWideString( const std::string& string );
			std::string ToNarrowString( const std::wstring& wstring );
#endif // _WIN32
		}
	};
}
