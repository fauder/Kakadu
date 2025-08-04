// Engine Includes.
#include "Math.hpp"
#include "Matrix.hpp"
#include "Polar.h"
#include "Vector.hpp"

namespace Engine::Math
{
/* Conversions Between Rotation Representations. */

	/* In row-major form. Results in counter-clockwise rotation.
	 * Describes the intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * equal to  the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	Engine::Matrix4x4 EulerToMatrix( Engine::Radians heading_around_y, Engine::Radians pitch_around_x, Engine::Radians bank_around_z )
	{
		const auto sin_pitch   = Math::Sin( pitch_around_x );
		const auto sin_heading = Math::Sin( heading_around_y );
		const auto sin_bank    = Math::Sin( bank_around_z );

		const auto cos_pitch   = Math::Cos( pitch_around_x );
		const auto cos_heading = Math::Cos( heading_around_y );
		const auto cos_bank    = Math::Cos( bank_around_z );

		const auto cos_bank_cos_heading  = cos_bank * cos_heading;
		const auto sin_pitch_sin_heading = sin_pitch * sin_heading;

		return Engine::Matrix4x4
		(
			{
				cos_bank_cos_heading + sin_bank * sin_pitch_sin_heading,		sin_bank * cos_pitch,		-cos_bank * sin_heading + sin_bank * sin_pitch * cos_heading,	0.0f,
				-sin_bank * cos_heading + cos_bank * sin_pitch_sin_heading,		cos_bank * cos_pitch,		sin_bank * sin_heading + sin_pitch * cos_bank_cos_heading,		0.0f,
				cos_pitch * sin_heading,										-sin_pitch,					cos_pitch * cos_heading,										0.0f,
				0.0f,															0.0f,						0.0f,															1.0f
			}
		);
	}

	/* In row-major form. Results in counter-clockwise rotation.
	 * Describes the intrinsic  (body-axis) rotation, in the order:	 heading (around y) -> pitch (around x) ->    bank (around z),
	 * equal to  the extrinsic (fixed-axis) rotation, in the order:		bank (around z) -> pitch (around x) -> heading (around y). */
	Engine::Matrix3x3 EulerToMatrix3x3( Engine::Radians heading_around_y, Engine::Radians pitch_around_x, Engine::Radians bank_around_z )
	{
		const auto sin_pitch   = Math::Sin( pitch_around_x );
		const auto sin_heading = Math::Sin( heading_around_y );
		const auto sin_bank    = Math::Sin( bank_around_z );

		const auto cos_pitch   = Math::Cos( pitch_around_x );
		const auto cos_heading = Math::Cos( heading_around_y );
		const auto cos_bank    = Math::Cos( bank_around_z );

		const auto cos_bank_cos_heading  = cos_bank * cos_heading;
		const auto sin_pitch_sin_heading = sin_pitch * sin_heading;

		return Engine::Matrix3x3
		(
			{
				cos_bank_cos_heading + sin_bank * sin_pitch_sin_heading,		sin_bank * cos_pitch,		-cos_bank * sin_heading + sin_bank * sin_pitch * cos_heading,
				-sin_bank * cos_heading + cos_bank * sin_pitch_sin_heading,		cos_bank * cos_pitch,		sin_bank * sin_heading + sin_pitch * cos_bank_cos_heading,	
				cos_pitch * sin_heading,										-sin_pitch,					cos_pitch * cos_heading
			}
		);
	}

/* Conversions Between Cartesian <=> Polar/Spherical Coordinates. */

	Polar2 ToPolar2( const Vector2& cartesian )
	{
		if( cartesian.IsZero() )
			return Polar2( ZERO_INITIALIZATION );

		return Polar2( cartesian.Magnitude(), Math::Atan2( cartesian.Y(), cartesian.X() ) );
	}

	Vector2 ToVector2( const Polar2& polar2 )
	{
		const auto r = polar2.Radius();

		if( IsZero( r ) )
			return Vector2( ZERO_INITIALIZATION );


		return Vector2( Math::Cos( polar2.Theta() ) * r, Math::Sin( polar2.Theta() ) * r );
	}

	Polar3_Spherical_Game ToPolar3_Spherical_Game( const Vector3& cartesian )
	{
		const auto x = cartesian.X(), y = cartesian.Y(), z = cartesian.Z();

		if( cartesian.IsZero() )
			return Polar3_Spherical_Game( ZERO_INITIALIZATION );

		const auto xz_projection_of_r = Math::Hypothenuse( x, z );

		return Polar3_Spherical_Game( cartesian.Magnitude(), Math::Atan2( x, z ), Math::Atan2( -y, xz_projection_of_r ) );
	}

	Vector3 ToVector3( const Polar3_Spherical_Game& polar3 )
	{
		const auto r = polar3.Radius();

		if( IsZero( r ) )
			return Vector3( ZERO_INITIALIZATION );

		const auto xz_projection_of_r = r * Math::Cos( polar3.Pitch() );

		return Vector3( xz_projection_of_r * Math::Sin( polar3.Heading() ), 
						r * -Math::Sin( polar3.Pitch() ),
						xz_projection_of_r * Math::Cos( polar3.Heading() ) );
	}
}