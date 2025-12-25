#pragma once

// Engine Includes.
#include "Macros.h"
#include "Concepts.h"

// std Includes.
#include <compare>

namespace Engine
{
	template< Concepts::IsEnum EnumType >
	class BitFlags
	{
		using Underlying = std::underlying_type_t< EnumType >;

	public:
		constexpr BitFlags()
			:
			flags( EnumType{ 0 } )
		{
		}

		constexpr BitFlags( const EnumType& flag )
			:
			flags( flag )
		{
		}

		constexpr BitFlags( std::same_as< EnumType > auto&& ... flags )
			:
			flags( EnumType( ( ( Underlying )flags | ... ) ) )
		{
		}

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( BitFlags );

		constexpr ~BitFlags() = default;

	/* Operator overloads: */

		constexpr bool operator ==( const BitFlags& ) const = default;
		constexpr bool operator !=( const BitFlags& ) const = default;
		constexpr auto operator<=>( const BitFlags& ) const = default;

		constexpr BitFlags& operator= ( const EnumType& flag )
		{
			Assign( flag );
			return *this;
		}

		constexpr BitFlags& operator= ( const Underlying& bit )
		{
			Assign( EnumType( bit ) );
			return *this;
		}

		constexpr bool operator== ( const EnumType& other_flag ) const
		{
			return flags == other_flag;
		}

	/* Setters: */

		/*
		 * Can not return reference to self in functions taking template parameter packs below because MSVC VS16.11 has a bug. Thus, all functions return void.
		 */

		constexpr void Assign( const EnumType& flag_to_assign )
		{
			flags = flag_to_assign;
		}

		constexpr void Assign( std::same_as< EnumType > auto&& ... flags_to_assign )
		{
			flags = EnumType( ( ( Underlying )flags_to_assign | ... ) );
		}

		constexpr void Reset()
		{
			flags = EnumType( 0 );
		}

		constexpr void Set( const EnumType& flag_to_set )
		{
			flags = EnumType( ( Underlying )flags | ( Underlying )flag_to_set );
		}

		constexpr void Set( std::same_as< EnumType > auto&& ... flags_to_set )
		{
			flags = EnumType( ( Underlying )flags | ( ( Underlying )flags_to_set | ... ) );
		}

		constexpr void Clear( const EnumType& flag_to_clear )
		{
			flags = EnumType( ( Underlying )flags & ~( Underlying )flag_to_clear );
		}

		constexpr void Clear( std::same_as< EnumType > auto&& ... flags_to_clear )
		{
			flags = EnumType( ( Underlying )flags & ~( ( Underlying )flags_to_clear | ... ) );
		}

	/* Queries: */

		constexpr bool IsSet( const EnumType& flag_to_inspect ) const
		{
			return ( Underlying )flags & Underlying( flag_to_inspect );
		}

		constexpr bool IsSet( std::same_as< EnumType > auto&& ... flags_to_inspect ) const
		{
			const auto parameters_combined = ( ( Underlying )flags_to_inspect | ... );
			return ( ( Underlying )flags & parameters_combined ) == parameters_combined;
		}

		constexpr Underlying ToBits() const { return ( Underlying )flags; }

	private:
		EnumType flags;
	};

}