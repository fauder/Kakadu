// Engine Includes.
#include "Math/Math.hpp"

namespace Natvis
{
/* Float: */
	header_function float SinFromCos( const float value )
	{
		return Kakadu::Math::SinFromCos( value );
	}

	header_function float Acos( const float value )
	{
		return ( float )Kakadu::Degrees( Kakadu::Math::Acos( value ) );
	}

	header_function float Asin( const float value )
	{
		return ( float )Kakadu::Degrees( Kakadu::Math::Asin( value ) );
	}

	header_function float Atan2( const float y, const float x )
	{
		return ( float )Kakadu::Degrees( Kakadu::Math::Atan2( y, x ) );
	}

/* Double: */

	header_function double SinFromCos( const double value )
	{
		return Kakadu::Math::SinFromCos( value );
	}

	header_function double Acos( const double value )
	{
		return ( double )Kakadu::DegreesD( Kakadu::Math::Acos( value ) );
	}

	header_function double Asin( const double value )
	{
		return ( double )Kakadu::DegreesD( Kakadu::Math::Asin( value ) );
	}

	header_function double Atan2( const double y, const double x )
	{
		return ( double )Kakadu::DegreesD( Kakadu::Math::Atan2( y, x ) );
	}

	/* Arbitrary (non-axis-aligned) rotation matrix decomposition, for Matrix3x3/Matrix4x4 natvis fallback display.
	 * "trace" is the sum of the 3 diagonal entries of the rotation/scale 3x3 block.
	 * The 6 off-diagonal parameters are the 3x3 block's entries at (0,1), (0,2), (1,0), (1,2), (2,0) & (2,1) respectively. */
	header_function float RotationAngleDegrees3x3( const float trace )
	{
		return ( float )Kakadu::Degrees( Kakadu::Math::Acos( Kakadu::Math::Clamp( ( trace - 1.0f ) / 2.0f, -1.0f, 1.0f ) ) );
	}

	header_function float RotationAxisX3x3( const float m01, const float m02, const float m10, const float m12, const float m20, const float m21 )
	{
		const float x      = m12 - m21;
		const float y      = m20 - m02;
		const float z      = m01 - m10;
		const float length = Kakadu::Math::Sqrt( x * x + y * y + z * z );

		return length > 0.0001f ? x / length : 0.0f;
	}

	header_function float RotationAxisY3x3( const float m01, const float m02, const float m10, const float m12, const float m20, const float m21 )
	{
		const float x      = m12 - m21;
		const float y      = m20 - m02;
		const float z      = m01 - m10;
		const float length = Kakadu::Math::Sqrt( x * x + y * y + z * z );

		return length > 0.0001f ? y / length : 0.0f;
	}

	header_function float RotationAxisZ3x3( const float m01, const float m02, const float m10, const float m12, const float m20, const float m21 )
	{
		const float x      = m12 - m21;
		const float y      = m20 - m02;
		const float z      = m01 - m10;
		const float length = Kakadu::Math::Sqrt( x * x + y * y + z * z );

		return length > 0.0001f ? z / length : 0.0f;
	}
	
	/* Since *actual* code never uses this namespace (only the .natvis does), we need to include this call somewhere. */
	header_function void ForceIncludeInBuild()
	{
/* Float: */
		SinFromCos( 1.0f );
		Acos( 1.0f );
		Asin( 1.0f );
		Atan2( 1.0f, 0.0f );

/* Double: */
		SinFromCos( 1.0 );
		Acos( 1.0 );
		Asin( 1.0 );
		Atan2( 1.0, 0.0 );

/* Rotation matrix decomposition: */
		RotationAngleDegrees3x3( 1.0f );
		RotationAxisX3x3( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
		RotationAxisY3x3( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
		RotationAxisZ3x3( 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f );
	}
}