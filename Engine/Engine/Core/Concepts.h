#pragma once

// std Includes.
#include <concepts>

namespace Engine::Concepts
{
	template< typename T > concept Pointer    = std::is_pointer_v< T >;
	template< typename T > concept NotPointer = not Pointer< T >;

	template< typename T, typename ... U > concept IsAnyOf = ( std::same_as< T, U > || ... );

	template< typename T > concept IsEnum = ( std::is_enum_v< T > );
}