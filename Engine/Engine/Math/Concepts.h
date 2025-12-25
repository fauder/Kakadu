#pragma once

// std Includes.
#include <concepts>

// TODO: Split into math concepts and non-math concepts.

namespace Engine::Concepts
{
	template< typename T > concept Arithmetic         = std::integral< T > || std::floating_point< T >;
	template< typename T > concept Arithmetic_NotBool = Arithmetic< T > && not std::same_as< T, bool >;

	template< typename T > concept Pointer    = std::is_pointer_v< T >;
	template< typename T > concept NotPointer = not Pointer< T >;

	template< std::size_t value > concept NonZero = ( value != 0 );

	template< typename T, typename ... U > concept IsAnyOf = ( std::same_as< T, U > || ... );

	template< typename T > concept IsEnum = ( std::is_enum_v< T > );
}