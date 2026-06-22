#pragma once

/* This macro is in its own file, so the std #include <compare> can be bundled with it. */

// std Includes.
#include <compare>

#define DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( class_name )\
	bool operator ==( const class_name& ) const = default;\
	bool operator !=( const class_name& ) const = default;\
	auto operator<=>( const class_name& ) const = default;

#define CONSTEXPR_DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( class_name )\
	constexpr bool operator ==( const class_name& ) const = default;\
	constexpr bool operator !=( const class_name& ) const = default;\
	constexpr auto operator<=>( const class_name& ) const = default;
