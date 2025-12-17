#pragma once

// std Includes.
#include <array>
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
			std::optional< std::string_view > ParseNextTokenAndAdvance( std::string_view& source, 
																		const std::string_view opening_delimiters = " \t", const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseTokenAndAdvance_WithPrefix( std::string_view& source,
																			   const std::string_view preceding_token,
																			   const std::string_view opening_delimiters = " \t", const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseTokenAndAdvance_WithPrefix( std::string_view& source,
																			   std::initializer_list< const std::string_view > preceding_tokens,
																			   const std::string_view opening_delimiters = " \t", const std::string_view closing_delimiters = " \t" );
			std::optional< std::string_view > ParseNextLineAndAdvance( std::string_view& source );
			void Replace( std::string& source, const std::string_view find_this, const std::string_view replace_with_this );

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

			/* Returns either the multiple splitted string views or the source string in case delimiter was never found. */
			std::vector< std::string_view > Split( std::string_view source, const char delimiter );

#ifdef _WIN32
			std::wstring ToWideString( const std::string& string );
			std::string ToNarrowString( const std::wstring& wstring );
#endif // _WIN32
		}
	};
}
