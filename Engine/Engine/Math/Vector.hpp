#pragma once

// Engine Includes.
#include "Core/Initialization.h"
#include "Core/Macros.h"
#include "Math/Concepts.h"
#include "Math/Math.hpp"
#include "Math/TypeTraits.h"

// std Includes.
#include <algorithm>

namespace Engine
{
	namespace Initialization
	{
		struct HomogeneousVectorInitialization {};
	}

	inline constexpr Initialization::HomogeneousVectorInitialization HOMOGENEOUS_VECTOR_INITIALIZATION;
}

namespace Engine::Math
{
	template< typename Component, std::size_t Size > requires( Size > 1 )
	class Vector
	{
		template< Concepts::Arithmetic, std::size_t RowSize, std::size_t ColumnSize >
			requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
		friend class Matrix;

		template< typename, std::size_t Size > requires( Size > 1 )
		friend class Vector; // For example, needed in the conversion constructor.

	public:
		using ComponentType = Component;

	public:
	/* Constructors. */
		constexpr Vector() : data{} {} // Same as the one with Initialization::ZeroInitialization parameter.

		constexpr Vector( Initialization::ZeroInitialization )
			: 
			data{}
		{
		} // Same as the default constructor.

		CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Vector );

		constexpr ~Vector() = default;

		constexpr explicit Vector( Initialization::UniformInitialization, const Component value )
		{
			std::fill_n( data, Size, value );
		}

#pragma warning(disable:26495) // Suppress "variable is uninitialized" warning, as not initializing it is the whole point of this constructor.
		constexpr explicit Vector( Initialization::NoInitialization )
		{}
#pragma warning(default:26495)

		constexpr explicit Vector( const Component x )
			:
			data{ x }
		{
		}

		constexpr Vector( const Component x, const Component y ) requires( Size >= 2 )
			:
			data{ x, y }
		{
		}

		constexpr Vector( const Component x, const Component y, const Component z ) requires( Size >= 3 )
			:
			data{ x, y, z }
		{
		}

		constexpr Vector( const Component x, const Component y, const Component z, const Component w ) requires( Size >= 4 )
			:
			data{ x, y, z, w }
		{
		}

		/* Conversion constructor: */
		template< typename OtherComponent >
		constexpr Vector( const Vector< OtherComponent, Size >& other_vector_of_different_type )
		{
			for( std::size_t i = 0; i < Size; ++i )
				data[ i ] = static_cast< Component >( other_vector_of_different_type.data[ i ] );
		}

		template< typename SmallerVectorType > requires( SmallerVectorType::Dimension() > 1 && SmallerVectorType::Dimension() < Size )
		constexpr Vector( const SmallerVectorType& vector_of_smaller_dimension )
			:
			data{} // This is needed, else the compiler complains about "failure was caused by a read of an uninitialized symbol".
		{
			std::copy( &vector_of_smaller_dimension[ 0 ], &vector_of_smaller_dimension[ 0 ] + SmallerVectorType::Dimension(), data );
		}

		template< typename SmallerVectorType > requires( Size == 4 && SmallerVectorType::Dimension() > 1 && SmallerVectorType::Dimension() < Size )
		constexpr Vector( const SmallerVectorType& vector_of_smaller_dimension, Initialization::HomogeneousVectorInitialization )
			:
			data{ Component( 0 ), Component( 0 ), Component( 0 ), Component( 1 ) }
		{
			std::copy( &vector_of_smaller_dimension[ 0 ], &vector_of_smaller_dimension[ 0 ] + SmallerVectorType::Dimension(), data );
		}

		template< typename ... Values >
		constexpr Vector( Values ... values ) requires( sizeof ... ( Values ) == Size )
			:
			data{ values ... }
		{
		}

	/* Comparison operators. */
		bool operator ==( const Vector& right_hand_side ) const
		{
			bool result = true;

			for( auto i = 0; i < Size; i++ )
				if constexpr( std::is_integral_v< Component > )
					result &= data[ i ] == right_hand_side.data[ i ];
				else
					result &= Math::IsEqual( data[ i ], right_hand_side.data[ i ] );

			return result;
		}

		bool operator !=( const Vector& right_hand_side ) const
		{
			return !operator==( right_hand_side );
		}

	/* Getters & Setters. */
		Component* Data() { return data; };
		const Component* Data() const { return data; };
		constexpr Component& operator[] ( const std::size_t index ) { return data[ index ]; }
		constexpr const Component& operator[] ( const std::size_t index ) const { return data[ index ]; }

		constexpr const Component& X() const requires( Size >= 1 ) { return data[ 0 ]; };
		constexpr const Component& Y() const requires( Size >= 2 ) { return data[ 1 ]; };
		constexpr const Component& Z() const requires( Size >= 3 ) { return data[ 2 ]; };
		constexpr const Component& W() const requires( Size >= 4 ) { return data[ 3 ]; };

		constexpr const Vector< Component, 2 >& XY()   const requires( Size >= 2 ) { return reinterpret_cast< const Vector< Component, 2 >& >( *this ); };
		constexpr const Vector< Component, 3 >& XYZ()  const requires( Size >= 3 ) { return reinterpret_cast< const Vector< Component, 3 >& >( *this ); };
		constexpr const Vector< Component, 4 >& XYZW() const requires( Size >= 4 ) { return reinterpret_cast< const Vector< Component, 4 >& >( *this ); };

		constexpr Vector< Component, 2 >& XY()		requires( Size >= 2 ) { return reinterpret_cast< Vector< Component, 2 >& >( *this ); };
		constexpr Vector< Component, 3 >& XYZ()		requires( Size >= 3 ) { return reinterpret_cast< Vector< Component, 3 >& >( *this ); };
		constexpr Vector< Component, 4 >& XYZW()	requires( Size >= 4 ) { return reinterpret_cast< Vector< Component, 4 >& >( *this ); };

		constexpr Vector& SetX( const Component value ) requires( Size >= 1 ) { data[ 0 ] = value; return *this; };
		constexpr Vector& SetY( const Component value ) requires( Size >= 2 ) { data[ 1 ] = value; return *this; };
		constexpr Vector& SetZ( const Component value ) requires( Size >= 3 ) { data[ 2 ] = value; return *this; };
		constexpr Vector& SetW( const Component value ) requires( Size >= 4 ) { data[ 3 ] = value; return *this; };

		template< typename... Values > requires( !std::is_pointer_v< Values > && ... )
		constexpr Vector& Set( Values ... values )
		{
			int i = 0;
			( /* Lambda: */ [ & ] { data[ i++ ] = values; }( ), ... ); // Utilize fold expressions with a lambda to "loop over" the parameter pack.

			return *this;
		}

		template< std::size_t OtherSize > requires( OtherSize > 1 && OtherSize <= Size )
		constexpr Vector& Set( const Component array_of_values[ OtherSize ] )
		{
			std::copy( array_of_values, array_of_values + OtherSize, data );
			return *this;
		}

		template< std::size_t OtherSize > requires( OtherSize > 1 && OtherSize <= Size )
		constexpr Vector& Set( const Component ( &array_of_values )[ OtherSize ] )
		{
			std::copy( array_of_values, array_of_values + OtherSize, data );
			return *this;
		}

	/* Arithmetic Operations: Unary operators. */
		constexpr Vector operator- () const
		{
			return *this * Component( -1 ); // Utilize operator * (scalar).
		}

	/* Arithmetic Operations: Binary operators (with a vector). */
		constexpr Vector operator+ ( const Vector& right_hand_side ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] += right_hand_side.data[ index ];
			
			return result;
		}

		constexpr Vector& operator+= ( const Vector& right_hand_side )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] += right_hand_side.data[ index ];
			

			return *this;
		}

		constexpr Vector operator- ( const Vector& right_hand_side ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] -= right_hand_side.data[ index ];
			
			return result;
		}

		constexpr Vector& operator-= ( const Vector& right_hand_side )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] -= right_hand_side.data[ index ];
			

			return *this;
		}

		constexpr Vector operator* ( const Vector& right_hand_side ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] *= right_hand_side.data[ index ];
			
			return result;
		}

		constexpr Vector& operator*= ( const Vector& right_hand_side )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] *= right_hand_side.data[ index ];
			

			return *this;
		}

		constexpr Vector operator/ ( const Vector& right_hand_side ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] /= right_hand_side.data[ index ];
			
			return result;
		}

		constexpr Vector& operator/= ( const Vector& right_hand_side )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] /= right_hand_side.data[ index ];
			

			return *this;
		}

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form vector-operator-scalar. */
		constexpr Vector operator+ ( const Component scalar ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] += scalar;

			return result;
		}

		constexpr Vector& operator+= ( const Component scalar )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] += scalar;

			return *this;
		}

		constexpr Vector operator- ( const Component scalar ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] -= scalar;

			return result;
		}

		constexpr Vector& operator-= ( const Component scalar )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] -= scalar;

			return *this;
		}

		constexpr Vector operator* ( const Component scalar ) const
		{
			Vector result( *this );
			for( auto index = 0; index < Size; index++ )
				result.data[ index ] *= scalar;

			return result;
		}

		constexpr Vector& operator*= ( const Component scalar )
		{
			for( auto index = 0; index < Size; index++ )
				data[ index ] *= scalar;

			return *this;
		}

		constexpr Vector operator/ ( const Component scalar ) const
		{
			Vector result( *this );
			if constexpr( std::is_integral_v< Component > )
			{
				for( auto index = 0; index < Size; index++ )
					result.data[ index ] /= scalar; // Divide directly as division of 1/scalar will give zero when scalar > 1.
			}
			else
			{
				const auto inverse_of_scalar = Component( 1 ) / scalar; // We can calculate the inverse safely.
				for( auto index = 0; index < Size; index++ )
					result.data[ index ] *= inverse_of_scalar;
			}

			return result;
		}

		constexpr Vector& operator/= ( const Component scalar )
		{
			if constexpr( std::is_integral_v< Component > )
				for( auto index = 0; index < Size; index++ )
				data[ index ] /= scalar; // Divide directly as division of 1/scalar will give zero when scalar > 1.
			else
			{
				const auto inverse_of_scalar = Component( 1 ) / scalar; // We can calculate the inverse safely.
				for( auto index = 0; index < Size; index++ )
					data[ index ] *= inverse_of_scalar;
			}

			return *this;
		}

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form scalar-operator-vector. */
		constexpr friend Vector operator + ( const Component scalar, const Vector& vector )
		{
			const Vector result( UNIFORM_INITIALIZATION, scalar );
			return result + vector; // Leverage already defined operator +( scalar ).
		}

		constexpr friend Vector operator - ( const Component scalar, const Vector& vector )
		{
			const Vector result( UNIFORM_INITIALIZATION, scalar );
			return result - vector; // Leverage already defined operator -( scalar ).
		}

		constexpr friend Vector operator * ( const Component scalar, const Vector& vector )
		{
			const Vector result( UNIFORM_INITIALIZATION, scalar );
			return result * vector; // Leverage already defined operator *( scalar ).
		}

		constexpr friend Vector operator / ( const Component scalar, const Vector& vector )
		{
			const Vector result( UNIFORM_INITIALIZATION, scalar );
			return result / vector; // Leverage already defined operator /( scalar ).
		}

	/* Other Queries. */
		static consteval std::size_t Dimension() { return Size; }

		static consteval Vector Zero()		{ return Vector{ ZERO_INITIALIZATION }; }
		static consteval Vector One()		{ return Vector{ UNIFORM_INITIALIZATION, Component( 1 ) }; }

		static consteval Vector Left()		requires( Size >= 1 ) { return Vector{ -Component( 1 ) }; }
		static consteval Vector Right()		requires( Size >= 1 ) { return Vector{ +Component( 1 ) }; }
		static consteval Vector Down()		requires( Size >= 2 ) { return Vector{ Component( 0 ), -Component( 1 ) }; }
		static consteval Vector Up()		requires( Size >= 2 ) { return Vector{ Component( 0 ), +Component( 1 ) }; }
		/* Using left-handed coordinate system. */
		static consteval Vector Backward()	requires( Size >= 3 ) { return Vector{ Component( 0 ), Component( 0 ), -Component( 1 ) }; }
		/* Using left-handed coordinate system. */
		static consteval Vector Forward()	requires( Size >= 3 ) { return Vector{ Component( 0 ), Component( 0 ), +Component( 1 ) }; }

		constexpr bool IsZero() const
		{
			if constexpr( std::is_integral_v< Component > )
			{
				if      constexpr( Size == 2 )
					return data[ 0 ] == 0 && data[ 1 ] == 0;
				else if constexpr( Size == 3 )
					return data[ 0 ] == 0 && data[ 1 ] == 0 && data[ 2 ] == 0;
				else if constexpr( Size == 4 )
					return data[ 0 ] == 0 && data[ 1 ] == 0 && data[ 2 ] == 0 && data[ 3 ] == 0;
				else
				{
					bool non_zero_encountered = false;
					for( auto i = 0; i < Size && !non_zero_encountered; i++ )
						non_zero_encountered |= data[ i ] != 0;

					return !non_zero_encountered;
				}
			}
			else // Floating point.
			{
				if      constexpr( Size == 2 )
					return Math::IsZero( data[ 0 ] ) && Math::IsZero( data[ 1 ] );
				else if constexpr( Size == 3 )
					return Math::IsZero( data[ 0 ] ) && Math::IsZero( data[ 1 ] ) && Math::IsZero( data[ 2 ] );
				else if constexpr( Size == 4 )
					return Math::IsZero( data[ 0 ] ) && Math::IsZero( data[ 1 ] ) && Math::IsZero( data[ 2 ] ) && Math::IsZero( data[ 3 ] );
				else
				{
					bool non_zero_encountered = false;
					for( auto i = 0; i < Size && !non_zero_encountered; i++ )
						non_zero_encountered |= !Math::IsZero( data[ i ] );

					return !non_zero_encountered;
				}
			}
		}

		constexpr bool IsNormalized() const requires( std::is_integral_v< Component > == false )
		{
			return Math::IsEqualSquared( MagnitudeSquared(), Component( 1 ) );
		}

		// TODO: Equality operator etc.

	/* Other Arithmetic Operations. */
		/* With self. */
		constexpr Component Dot() const
		{
			Component result( 0 );

			for( auto index = 0; index < Size; index++ )
				result += data[ index ] * data[ index ];

			return result;
		}

		// This should not be needed for vectors other than 3D & 4D in theory, but no need to restrict.
		template< Concepts::Arithmetic Component_, std::size_t Size_ > // Have to use different template parameters here because C++...
		friend constexpr Component_ Dot( const Vector< Component_, Size_ >& u, const Vector< Component_, Size_ >& v );

		// Cross product is only defined for vectors of 3 & 7 dimensions apparently, but practically we only need it for 3D.
		template< Concepts::Arithmetic Component_ > // Have to use different template parameters here because C++...
		friend constexpr Vector< Component_, 3 > Cross( const Vector< Component_, 3 >& u, const Vector< Component_, 3 >& v );

		constexpr Component MagnitudeSquared() const { return Dot(); }
		Component Magnitude() const requires( std::floating_point< Component > ) { return Math::Sqrt( MagnitudeSquared() ); }

		Vector Normalized() const requires( std::floating_point< Component > )
		{
			if( const auto magnitude = Magnitude(); magnitude > TypeTraits< Component >::Epsilon() )
			{
				const Component one_over_magnitude = Component( 1 ) / Magnitude();
				return *this * one_over_magnitude;
			}

			return { ZERO_INITIALIZATION };
		}

		Vector& Normalize() requires std::floating_point< Component >
		{
			const Component one_over_magnitude = Component( 1 ) / Magnitude();
			*this *= one_over_magnitude;
			return *this;
		}

	protected:
		Component data[ Size ];
	};

	// This should not be needed for vectors other than 3D & 4D in theory, but no need to restrict.
	template< Concepts::Arithmetic Component, std::size_t Size >
	constexpr Component Dot( const Vector< Component, Size >& u, const Vector< Component, Size >& v ) 
	{
		Component result( 0 );

		for( auto index = 0; index < Size; index++ )
			result += u.data[ index ] * v.data[ index ];

		return result;
	}

	// Cross product is only defined for vectors of 3 & 7 dimensions apparently, but practically we only need it for 3D.
	template< Concepts::Arithmetic Component >
	constexpr Vector< Component, 3 > Cross( const Vector< Component, 3 >& u, const Vector< Component, 3 >& v )
	{
		// u X v = ( u2v3 - u3v2, u3v1 - u1v3, u1v2 - u2v1 )
		return Vector< Component, 3 >( u.data[ 1 ] * v.data[ 2 ] - u.data[ 2 ] * v.data[ 1 ],
									   u.data[ 2 ] * v.data[ 0 ] - u.data[ 0 ] * v.data[ 2 ],
									   u.data[ 0 ] * v.data[ 1 ] - u.data[ 1 ] * v.data[ 0 ] );
	}

	template< Concepts::Arithmetic Component, std::size_t Size >
	constexpr Vector< Component, Size > Lerp( const Vector< Component, Size > vector_a, const Vector< Component, Size >& vector_b, const Component t )
	{
		return ( Component{ 1 } - t ) * vector_a + t * vector_b;
	}
}

namespace Engine
{
	using Vector2  = Math::Vector< float,			2 >;
	using Vector3  = Math::Vector< float,			3 >;
	using Vector4  = Math::Vector< float,			4 >;
	using Vector2D = Math::Vector< double,			2 >;
	using Vector3D = Math::Vector< double,			3 >;
	using Vector4D = Math::Vector< double,			4 >;
	using Vector2I = Math::Vector< int,				2 >;
	using Vector3I = Math::Vector< int,				3 >;
	using Vector4I = Math::Vector< int,				4 >;
	using Vector2U = Math::Vector< unsigned int,	2 >;
	using Vector3U = Math::Vector< unsigned int,	3 >;
	using Vector4U = Math::Vector< unsigned int,	4 >;
	using Vector2B = Math::Vector< bool,			2 >;
	using Vector3B = Math::Vector< bool,			3 >;
	using Vector4B = Math::Vector< bool,			4 >;

	namespace Concepts
	{
		template< typename >
		struct IsVectorTag : public std::false_type {};

		template< typename T, std::size_t S >
		struct IsVectorTag< Math::Vector< T, S > > : public std::true_type {};

		template< typename Type > concept IsVector = bool( IsVectorTag< Type >::value );
	}
}
