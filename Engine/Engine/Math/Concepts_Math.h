#pragma once

// std Includes.
#include <concepts>

namespace Engine::Concepts
{
	template< typename T > concept Arithmetic         = std::integral< T > || std::floating_point< T >;
	template< typename T > concept Arithmetic_NotBool = Arithmetic< T > && not std::same_as< T, bool >;

	template< std::size_t value > concept NonZero = ( value != 0 );
}