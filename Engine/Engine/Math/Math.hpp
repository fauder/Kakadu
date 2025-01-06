#pragma once

// Engine Includes.
#include "Angle.hpp"
#include "Concepts.h"

// std Includes.
#include <cmath>

/* Forward Declarations. */
namespace Engine::Math
{
	class Polar2;
	class Polar3_Spherical_Game;

	template< typename Component, std::size_t Size > requires( Size > 1 ) 
	class Vector;

	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
	class Matrix;
}

namespace Engine
{
	using Vector2 = Math::Vector< float, 2 >;
	using Vector3 = Math::Vector< float, 3 >;

	using Matrix3x3 = Math::Matrix< float, 3, 3 >;
	using Matrix4x4 = Math::Matrix< float, 4, 4 >;
}

// TODO: Make suitable functions constexpr.

namespace Engine::Math
{
/* Arithmetic. */
	template< std::floating_point Value >
	Value SquareOf( Value value ) { return std::pow( value, Value{ 2 } ); }

	template< std::floating_point Value >
	Value Abs( const Value value ) { return std::abs( value ); }

	template< typename Value >
	Value Min( const Value left, const Value right ) { return left < right ? left : right; }

	template< typename Value >
	Value Max( const Value left, const Value right ) { return left > right ? left : right; }

	template< Concepts::Arithmetic Value >
	auto Sqrt( const Value value ) { return std::sqrt( value ); }

	template< std::floating_point Value >
	Value Pow( const Value value, const Value exponent ) { return std::pow( value, exponent ); }

	template< std::floating_point Value >
	Value Exp( const Value exponent ) { return std::exp( exponent ); }

	template< Concepts::Arithmetic Value >
	Value Round( const Value value ) { return std::round( value ); }

	template< typename Value, std::floating_point PercentType >
	Value Lerp( const Value value_a, const Value value_b, const PercentType t ) { return ( PercentType( 1 ) - t ) * value_a + t * value_b; }

	template< std::totally_ordered Value >
	[[ nodiscard( "Clamped value is not assigned back to any variable." ) ]]
	Value Clamp( const Value value, const Value minimum, const Value maximum ) { return value < minimum ? minimum : value > maximum ? maximum : value; }

	template< typename Component, std::size_t Size > requires( Size > 1 )
	Component Distance( const Vector< Component, Size >& vector_a, const Vector< Component, Size >& vector_b )
	{
		return ( vector_a - vector_b ).Magnitude();
	}

	int RoundToMultiple_PowerOf2( const int value, const int multiple );

/* Trigonometry. */
	template< std::floating_point Value >
	Value Hypothenuse( const Value x, const Value y ) { return std::sqrt( x * x + y * y ); }

	template< std::floating_point Value >
	Value NonHyptothenuseEdge( const Value edge, const Value hyptothenuse ) { return std::sqrt( hyptothenuse * hyptothenuse - edge * edge ); }

	template< std::floating_point Value >
	Value Sin( const Radians< Value > angle ) { return std::sin( Value( angle ) ); }

	template< std::floating_point Value >
	Value Cos( const Radians< Value > angle ) { return std::cos( Value( angle ) ); }

	template< std::floating_point Value >
	Value SinFromCos( const Value cos ) { return std::sqrt( Value( 1 ) - cos * cos ); }

	template< std::floating_point Value >
	Value CosFromSin( const Value sin ) { return std::sqrt( Value( 1 ) - sin * sin ); }

	template< std::floating_point Value >
	Value Tan( const Radians< Value > angle ) { return std::tan( Value( angle ) ); }

	template< std::floating_point Value >
	Radians< Value > Acos( const Value cosine ) { return Radians< Value >( std::acos( cosine ) ); }

	template< std::floating_point Value >
	Radians< Value > Asin( const Value sine ) { return Radians< Value >( std::asin( sine ) ); }

	template< std::floating_point Value >
	Radians< Value > Atan( const Value slope ) { return Radians< Value >( std::atan( slope ) ); }

	template< std::floating_point Value >
	Radians< Value > Atan2( const Value y, const Value x ) { return Radians< Value >( std::atan2( y, x ) ); }

	template< Concepts::Arithmetic Value, std::size_t Size >
	Radians< Value > Angle( const Vector< Value, Size >& a, const Vector< Value, Size >& b )
	{
		ASSERT_DEBUG_ONLY( a.IsNormalized() && R"(Math::Angle(): The vector "a" is not normalized!)" );
		ASSERT_DEBUG_ONLY( b.IsNormalized() && R"(Math::Angle(): The vector "b" is not normalized!)" );

		return Math::Acos( Math::Clamp( Dot( a, b ), Value( -1 ), Value( +1 ) ) );
	}

/* Conversions Between Rotation Representations. */

	/* In row-major form. Results in counter-clockwise rotation.
	 * Describes the intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * equal to  the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	Engine::Matrix4x4 EulerToMatrix( Engine::Radians heading_around_y, Engine::Radians pitch_around_x, Engine::Radians bank_around_z );
	/* In row-major form. Results in counter-clockwise rotation.
	 * Describes the intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * equal to  the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	Engine::Matrix3x3 EulerToMatrix3x3( Engine::Radians heading_around_y, Engine::Radians pitch_around_x, Engine::Radians bank_around_z );
	
	/* In-place modification of the upper-left 3x3 portion.
	 *
	 * In row-major form. Results in counter-clockwise rotation.
	 * Describes the intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * equal to  the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	template< Concepts::Arithmetic Type, std::size_t MatrixSize >
	void EulerToMatrix( Matrix< Type, MatrixSize, MatrixSize >& matrix, Engine::Radians heading_around_y, Engine::Radians pitch_around_x, Engine::Radians bank_around_z ) requires( MatrixSize >= 3 )
	{
		const auto sin_pitch   = Math::Sin( pitch_around_x );
		const auto sin_heading = Math::Sin( heading_around_y );
		const auto sin_bank    = Math::Sin( bank_around_z );

		const auto cos_pitch   = Math::Cos( pitch_around_x );
		const auto cos_heading = Math::Cos( heading_around_y );
		const auto cos_bank    = Math::Cos( bank_around_z );

		const auto cos_bank_cos_heading = cos_bank * cos_heading;
		const auto sin_pitch_sin_heading = sin_pitch * sin_heading;

		matrix[ 0 ][ 0 ] = cos_bank_cos_heading + sin_bank * sin_pitch_sin_heading;		matrix[ 0 ][ 1 ] = sin_bank * cos_pitch;	matrix[ 0 ][ 2 ] = -cos_bank * sin_heading + sin_bank * sin_pitch * cos_heading;
		matrix[ 1 ][ 0 ] = -sin_bank * cos_heading + cos_bank * sin_pitch_sin_heading;	matrix[ 1 ][ 1 ] = cos_bank * cos_pitch;	matrix[ 1 ][ 2 ] = sin_bank * sin_heading + sin_pitch * cos_bank_cos_heading;	
		matrix[ 2 ][ 0 ] = cos_pitch * sin_heading;										matrix[ 2 ][ 1 ] = -sin_pitch;				matrix[ 2 ][ 2 ] = cos_pitch * cos_heading;									
	}

	/* Expects matrix in row-major form. Counter-clockwise rotation.
	 * The matrix should describe  an intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * which would be the same as the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	template< Concepts::Arithmetic Type, std::size_t MatrixSize >
	void MatrixToEuler( const Matrix< Type, MatrixSize, MatrixSize >& matrix, Engine::Radians& heading_around_y, Engine::Radians& pitch_around_x, Engine::Radians& bank_around_z ) requires( MatrixSize >= 3 )
	{
		/* The matrix' values for reference:
		{
			matrix[ 0 ][ 0 ] = cos_bank_cos_heading + sin_bank * sin_pitch_sin_heading		matrix[ 0 ][ 1 ] = sin_bank * cos_pitch		matrix[ 0 ][ 2 ] = -cos_bank * sin_heading + sin_bank * sin_pitch * cos_heading
			matrix[ 1 ][ 0 ] = -sin_bank * cos_heading + cos_bank * sin_pitch_sin_heading	matrix[ 1 ][ 1 ] = cos_bank * cos_pitch		matrix[ 1 ][ 2 ] = sin_bank * sin_heading + sin_pitch * cos_bank_cos_heading
			matrix[ 2 ][ 0 ] = cos_pitch * sin_heading										matrix[ 2 ][ 1 ] = -sin_pitch				matrix[ 2 ][ 2 ] = cos_pitch * cos_heading
		}*/

		using namespace Math::Literals;

		const auto sin_pitch = -matrix[ 2 ][ 1 ];

		if( sin_pitch <= -1.0f )
			pitch_around_x = Radians( -Constants< float >::Pi_Over_Two() );
		else if( sin_pitch >= +1.0f )
			pitch_around_x = Radians( Constants< float >::Pi_Over_Two() );
		else
			pitch_around_x = Math::Asin( sin_pitch );

		const auto cos_pitch = Math::Cos( pitch_around_x );

		if( Math::IsZero( cos_pitch ) ) // Which means pitch is +90 or -90 -> gimbal lock!
		{
			// Set bank to zero & assign all rotation around the vertical axis to heading.
			bank_around_z = 0.0_rad;

			// Since bank = 0, it means that cos_bank = 1 & sin_bank = 0. Reminder: cos_pitch is also 0.

			/* Updated matrix' values for reference:
			{
				matrix[ 0 ][ 0 ] = cos_heading					matrix[ 0 ][ 1 ] = 0				matrix[ 0 ][ 2 ] = -sin_heading
				matrix[ 1 ][ 0 ] = sin_pitch_sin_heading		matrix[ 1 ][ 1 ] = 0				matrix[ 1 ][ 2 ] = sin_pitch * cos_heading
				matrix[ 2 ][ 0 ] = 0							matrix[ 2 ][ 1 ] = -sin_pitch		matrix[ 2 ][ 2 ] = 0
			}*/

			heading_around_y = Math::Atan2( -matrix[ 0 ][ 2 ], matrix[ 0 ][ 0 ] );
		}
		else
		{
			/*
			const auto sin_heading = matrix[ 2 ][ 0 ] / cos_pitch;
			const auto cos_heading = matrix[ 2 ][ 2 ] / cos_pitch;
			*/

			/* Since cos_pitch is a common denominator between these two terms, we can directly pass the numerators of these fractions to atan2() instead. */

			heading_around_y = Math::Atan2( matrix[ 2 ][ 0 ], matrix[ 2 ][ 2 ] );

			/* Similar method for bank: */

			bank_around_z = Math::Atan2( matrix[ 0 ][ 1 ], matrix[ 1 ][ 1 ] );
		}
	}

	/* Expects matrix in row-major form. Counter-clockwise rotation.
	 * The matrix should describe  an intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * which would be the same as the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	template< Concepts::Arithmetic Type, std::size_t MatrixSize >
	void MatrixToEuler( const Matrix< Type, MatrixSize, MatrixSize >& matrix, Engine::Degrees& heading_around_y, Engine::Degrees& pitch_around_x, Engine::Degrees& bank_around_z ) requires( MatrixSize >= 3 )
	{
		Engine::Radians heading( heading_around_y ), pitch( pitch_around_x ), bank( bank_around_z );

		MatrixToEuler( matrix, heading, pitch, bank );

		heading_around_y = Engine::Degrees( heading );
		pitch_around_x   = Engine::Degrees( pitch );
		bank_around_z    = Engine::Degrees( bank );
	}

/* Conversions Between Cartesian, Polar, Cylindrical & Spherical Coordinates. */
	Polar2 ToPolar2( const Vector2& cartesian );
	Vector2 ToVector2( const Polar2& polar2 );
	Polar3_Spherical_Game ToPolar3_Spherical_Game( const Vector3& cartesian );
	Vector3 ToVector3( const Polar3_Spherical_Game& polar3 );
}