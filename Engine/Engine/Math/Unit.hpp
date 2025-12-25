#pragma once

// Engine Includes.
#include "Core/Initialization.h"

#include "Math/Concepts_Math.h"
#include "Math/TypeTraits.h"

// std Includes.
#include <compare>

namespace Engine
{
	template< Concepts::Arithmetic Type, 
			  template< class > class Derived > /* Explicitly restrict to template classes */
	class Unit
	{
	private:
		using DerivedType = Derived< Type >;

	public:
		using UnderlyingType = Type;

	public:
	/* Constructors. */
		constexpr Unit() : value( Type( 0 ) ) {}
		constexpr Unit( const Unit< Type, Derived >& other ) = default;

		explicit Unit( Initialization::NoInitialization ) {}

		/* Construct from underlying type. */
		constexpr explicit Unit( Type value ) : value( value ) {}

		constexpr ~Unit() = default;

	/* Queries. */
		constexpr const Type& Value() const { return value; }

	/* Comparison Operators. */
		constexpr auto operator<=> ( const Unit< Type, Derived >& ) const = default;

		bool operator== ( const DerivedType& right_hand_side ) const
		{
			if constexpr( std::is_integral_v< Type > )
				return value == right_hand_side.value;
			else
				return Math::IsEqual( value, right_hand_side.value );
		}

		bool operator!= ( const DerivedType& right_hand_side ) const
		{
			return !operator== ( right_hand_side );
		}

	/* Arithmetic Operations: Unary operators. */
		/* Return the value as-is. */
		constexpr DerivedType operator+ () const { return ( DerivedType& )*this;
		}
		constexpr DerivedType operator- () const requires( std::is_floating_point_v< Type > || std::is_signed_v< Type > )
		{
			return DerivedType( Type( -1 ) * value );
		}

	/* Arithmetic Operations: Binary operators (with an Unit). */
		constexpr DerivedType operator+ ( const DerivedType right_hand_side ) const
		{
			return DerivedType{ value + right_hand_side.value };
		}

		constexpr DerivedType& operator+= ( const DerivedType right_hand_side )
		{
			value += right_hand_side.value;

			return ( DerivedType& )*this;
		}

		constexpr DerivedType operator- ( const DerivedType right_hand_side ) const
		{
			return DerivedType{ value - right_hand_side.value };
		}

		constexpr DerivedType& operator-= ( const DerivedType right_hand_side )
		{
			value -= right_hand_side.value;

			return ( DerivedType& )*this;
		}

		/* No operators * and *= are NOT defined on purpose; This is because the multiplication of 2 Units would return UnitSquared (or some other name), which is clearly not of type Unit. */

		constexpr DerivedType operator/ ( const DerivedType right_hand_side ) const
		{
			DerivedType result( *this );
			if constexpr( std::is_integral_v< Type > )
				result.value /= right_hand_side.value; // Divide directly as division of 1/right_hand_side will give zero when right_hand_side > 1.
			else
			{
				const auto inverse_of_right_hand_side = Type( 1 ) / right_hand_side.value; // We can calculate the inverse safely.
				result.value *= inverse_of_right_hand_side;
			}

			return result;
		}

		constexpr DerivedType& operator/= ( const DerivedType right_hand_side )
		{
			if constexpr( std::is_integral_v< Type > )
				value /= right_hand_side.value; // Divide directly as division of 1/right_hand_side will give zero when right_hand_side > 1.
			else
			{
				const auto inverse_of_right_hand_side = Type( 1 ) / right_hand_side.value; // We can calculate the inverse safely.
				value *= inverse_of_right_hand_side;
			}

			return ( DerivedType& )*this;
		}

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form unit-operator-scalar. */
		constexpr DerivedType operator+ ( const Type right_hand_side ) const
		{
			return DerivedType{ value + right_hand_side };
		}

		constexpr DerivedType operator+= ( const Type right_hand_side )
		{
			value += right_hand_side;
			return ( DerivedType& )*this;
		}

		constexpr DerivedType operator- ( const Type right_hand_side ) const
		{
			return DerivedType{ value - right_hand_side };
		}

		constexpr DerivedType operator-= ( const Type right_hand_side )
		{
			value -= right_hand_side;
			return ( DerivedType& )*this;
		}

		constexpr DerivedType operator* ( const Type right_hand_side ) const
		{
			return DerivedType{ value * right_hand_side };
		}

		constexpr DerivedType operator*= ( const Type right_hand_side )
		{
			value *= right_hand_side;
			return ( DerivedType& )*this;
		}

		constexpr DerivedType operator/ ( const Type right_hand_side ) const
		{
			return DerivedType{ value / right_hand_side };
		}

		constexpr DerivedType operator/= ( const Type right_hand_side )
		{
			value /= right_hand_side;
			return ( DerivedType& )*this;
		}

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form scalar-operator-unit. */
		template< Concepts::Arithmetic Type_, template< class > class Derived_ > // Have to use a different template parameter here because C++...
		friend constexpr Derived_< Type_ > operator+ ( const Type_ left_hand_side_value, const Derived_< Type_ >& right_hand_side_unit );

		template< Concepts::Arithmetic Type_, template< class > class Derived_ > // Have to use a different template parameter here because C++...
		friend constexpr Derived_< Type_ > operator- ( const Type_ left_hand_side_value, const Derived_< Type_ >& right_hand_side_unit );

		template< Concepts::Arithmetic Type_, template< class > class Derived_ > // Have to use a different template parameter here because C++...
		friend constexpr Derived_< Type_ > operator* ( const Type_ left_hand_side_value, const Derived_< Type_ >& right_hand_side_unit );

		template< Concepts::Arithmetic Type_, template< class > class Derived_ > // Have to use a different template parameter here because C++...
		friend constexpr Derived_< Type_ > operator/ ( const Type_ left_hand_side_value, const Derived_< Type_ >& right_hand_side_unit );

	/* Other Operators. */
		constexpr explicit operator Type() const { return value; }

	protected:
		Type value;
	};

	template< Concepts::Arithmetic Type, template< class > class Derived >
	constexpr Derived< Type > operator+ ( const Type left_hand_side_value, const Derived< Type >& right_hand_side_unit )
	{
		return Derived< Type >( left_hand_side_value + right_hand_side_unit.value );
	}

	template< Concepts::Arithmetic Type, template< class > class Derived >
	constexpr Derived< Type > operator- ( const Type left_hand_side_value, const Derived< Type >& right_hand_side_unit )
	{
		return Derived< Type >( left_hand_side_value - right_hand_side_unit.value );
	}

	template< Concepts::Arithmetic Type, template< class > class Derived >
	constexpr Derived< Type > operator* ( const Type left_hand_side_value, const Derived< Type >& right_hand_side_unit )
	{
		return Derived< Type >( left_hand_side_value * right_hand_side_unit.value );
	}

	template< Concepts::Arithmetic Type, template< class > class Derived >
	constexpr Derived< Type > operator/ ( const Type left_hand_side_value, const Derived< Type >& right_hand_side_unit )
	{
		return Unit< Type, Derived >( left_hand_side_value / right_hand_side_unit.value );
	}
}