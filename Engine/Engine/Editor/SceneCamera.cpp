// Engine Includes.
#include "SceneCamera.h"
#include "Core/ImGuiDrawer.hpp"
#include "Core/ImGuiUtility.h"
#include "Core/Platform.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <ImGui.h>

using namespace Kakadu::Math::Literals;

namespace Kakadu::Editor
{
	SceneCamera::SceneCamera()
		:
		animation_is_enabled( false ),
		camera( &transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Kakadu::Constants< Radians >::Pi_Over_Two(), Platform::GetAspectRatio() ) ),
		rotation_speed( 5.0f ),
		move_speed( 5.0f ),
		controller_flight( &camera, rotation_speed )
	{
		Reset();
	}

	void SceneCamera::Update( const float current_time, const float delta_time, const bool mouse_controls_the_camera )
	{
		const Radians current_time_as_angle( current_time );

		if( animation_is_enabled )
		{
			/* Orbit motion: */

			Kakadu::Math::Vector< Radians, 3 > old_euler_angles;
			Kakadu::Math::QuaternionToEuler( transform.GetRotation(), old_euler_angles );
			// Don't modify X & Z euler angles; Allow the user to modify them.
			transform.SetRotation( Kakadu::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, old_euler_angles.X(), old_euler_angles.Z() ) );

			auto new_pos = rotation_center - ( transform.Forward() * animation_orbit_radius );
			new_pos.SetY( transform.GetTranslation().Y() ); // Don't modify Y position; Allow the user to modify it.
			transform.SetTranslation( new_pos );
		}
		else
		{
			if( mouse_controls_the_camera )
			{
				// Control via mouse:
				const auto [ mouse_x_delta_pos, mouse_y_delta_pos ] = Platform::GetMouseCursorDeltas();
				controller_flight
					.OffsetHeading( Radians( +mouse_x_delta_pos ) )
					.OffsetPitch( Radians( +mouse_y_delta_pos ), -( Kakadu::Constants< Radians >::Pi_Over_Two() - 0.01_rad ), Kakadu::Constants< Radians >::Pi_Over_Two() - 0.01_rad );
			}
		}

		if( Platform::IsKeyPressed( Platform::KeyCode::KEY_W ) )
			transform.OffsetTranslation( transform.Forward() * +move_speed * delta_time );
		if( Platform::IsKeyPressed( Platform::KeyCode::KEY_S ) )
			transform.OffsetTranslation( transform.Forward() * -move_speed * delta_time );
		if( Platform::IsKeyPressed( Platform::KeyCode::KEY_A ) )
			transform.OffsetTranslation( transform.Right() * -move_speed * delta_time );
		if( Platform::IsKeyPressed( Platform::KeyCode::KEY_D ) )
			transform.OffsetTranslation( transform.Right() * +move_speed * delta_time );
	}

	void SceneCamera::Reset()
	{
		animation_orbit_radius = 30.0f;

		ResetProjection();

		SwitchView( View::FRONT );
	}

	void SceneCamera::ResetProjection()
	{
		camera = Kakadu::Camera( &transform, camera.GetAspectRatio(), camera.GetVerticalFieldOfView() ); // Keep current aspect ratio & v-fov.
	}

	void SceneCamera::SwitchView( const View view )
	{
		switch( view )
		{
			case View::FRONT:
				transform.SetTranslation( 0.0f, 10.0f, -20.0f );
				transform.LookAt( Vector3::Forward() );
				break;
			case View::BACK:
				transform.SetTranslation( 0.0f, 10.0f, +20.0f );
				transform.LookAt( Vector3::Backward() );
				break;
			case View::LEFT:
				transform.SetTranslation( -10.0f, 10.0f, 0.0f );
				transform.LookAt( Vector3::Right() );
				break;
			case View::RIGHT:
				transform.SetTranslation( +10.0f, 10.0f, 0.0f );
				transform.LookAt( Vector3::Left() );
				break;
			case View::TOP:
				transform.SetTranslation( 0.0f, 60.0f, 0.0f );
				transform.LookAt( Vector3::Down() );
				break;
			case View::BOTTOM:
				transform.SetTranslation( 0.0f, -20.0f, 0.0f );
				transform.LookAt( Vector3::Up() );
				break;


			case View::CUSTOM_1:
				transform.SetTranslation( 12.0f, 10.0f, -14.0f );
				transform.SetRotation( -35.0_deg, 0.0_deg, 0.0_deg );
				break;

			default:
				break;
		}

		controller_flight.ResetToTransform();
	}

	Radians SceneCamera::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view, const float aspect_ratio ) const
	{
		return 2.0f * Kakadu::Math::Atan2( Kakadu::Math::Tan( horizontal_field_of_view / 2.0f ), aspect_ratio );
	}

	void SceneCamera::RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels )
	{
		camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
		camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Kakadu::Constants< Radians >::Pi_Over_Two(), camera.GetAspectRatio() ) );
	}

	void SceneCamera::RecalculateProjectionParameters( const Vector2I new_size_pixels )
	{
		RecalculateProjectionParameters( new_size_pixels.X(), new_size_pixels.Y() );
	}
}