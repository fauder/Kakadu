// Engine Includes.
#include "CameraController_Flight.h"
#include "Math/Math.hpp"

namespace Kakadu
{
	using namespace Math::Literals;

	CameraController_Flight::CameraController_Flight( Camera* camera, const float move_speed )
		:
		orientation_spherical( 1.0f, 0_rad, 0_rad ), // Unit-sphere used as these spherical coordinates are used as an orientation & the look-at vector is computed from it.
		move_speed( move_speed ),
		camera( camera )
	{
	}

	CameraController_Flight& CameraController_Flight::Invert()
	{
		camera->SetLookRotation( -camera->Forward() );
		ResetToTransform();
		return *this;
	}

	CameraController_Flight& CameraController_Flight::SetHeading( const Radians new_heading )
	{
		orientation_spherical.Heading() = new_heading;
		RecalculateRotationFromSphericalCoordinates();
		return *this;
	}

	CameraController_Flight& CameraController_Flight::SetHeading( const Radians new_heading, const Radians clamp_minimum, const Radians clamp_maximum )
	{
		SetHeading( Math::Clamp( new_heading, clamp_minimum, clamp_maximum ) );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::OffsetHeading( const Radians delta )
	{
		SetHeading( orientation_spherical.Heading() + delta );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::OffsetHeading( const Radians delta, const Radians clamp_minimum, const Radians clamp_maximum )
	{
		SetHeading( Math::Clamp( orientation_spherical.Heading() + delta, clamp_minimum, clamp_maximum ) );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::InvertHeading()
	{
		return OffsetHeading( Constants< Radians >::Pi() );
	}

	CameraController_Flight& CameraController_Flight::SetPitch( const Radians new_pitch )
	{
		orientation_spherical.Pitch() = new_pitch;
		RecalculateRotationFromSphericalCoordinates();
		return *this;
	}

	CameraController_Flight& CameraController_Flight::SetPitch( const Radians new_pitch, const Radians clamp_minimum, const Radians clamp_maximum )
	{
		SetPitch( Math::Clamp( new_pitch, clamp_minimum, clamp_maximum ) );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::OffsetPitch( const Radians delta )
	{
		SetPitch( orientation_spherical.Pitch() + delta );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::OffsetPitch( const Radians delta, const Radians clamp_minimum, const Radians clamp_maximum )
	{
		SetPitch( Math::Clamp( orientation_spherical.Pitch() + delta, clamp_minimum, clamp_maximum ) );
		return *this;
	}

	CameraController_Flight& CameraController_Flight::InvertPitch()
	{
		SetPitch( -orientation_spherical.Pitch() );
		return *this;
	}

	void CameraController_Flight::ResetToTransform()
	{
		orientation_spherical = Math::ToPolar3_Spherical_Game( camera->Forward() );
	}

	void CameraController_Flight::RecalculateRotationFromSphericalCoordinates()
	{
		camera->SetLookRotation( Math::ToVector3( orientation_spherical ) );
	}
}