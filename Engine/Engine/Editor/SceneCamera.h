#pragma once

// Engine Includes.
#include "Scene/Camera.h"
#include "Scene/CameraController_Flight.h"

namespace Engine::Editor
{
	struct SceneCamera
	{
		SceneCamera();

		enum class View
		{
			FRONT,
			BACK,
			LEFT,
			RIGHT,
			TOP,
			BOTTOM,

			CUSTOM_1,
		};

		void RenderImGui( const Vector2I viewport_resolution );

		void Update( const float current_time, const float delta_time, const bool mouse_controls_the_camera );

		header_function void SetRotationCenter( const Vector3& point ) { rotation_center = point; }

		void Reset();
		void ResetProjection();
		void SwitchView( const View view );

		Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view, const float aspect_ratio ) const;
		void RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels );
		void RecalculateProjectionParameters( const Vector2I new_size_pixels ); // Convenience overload.

		bool animation_is_enabled;
		/* 3 bytes(s) of padding. */

		float animation_orbit_radius;

		Engine::Transform transform;

		Engine::Camera camera;
		float rotation_speed;
		float move_speed;
		Vector3 rotation_center;
		/* 4 bytes(s) of padding. */

		Engine::CameraController_Flight controller_flight;
	};
}
