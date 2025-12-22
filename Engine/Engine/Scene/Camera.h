#pragma once

// Engine Includes.
#include "Transform.h"
#include "Math/Matrix.h"

namespace Engine
{
	class Camera
	{
	public:
		Camera( Transform* const transform, float aspect_ratio, Radians vertical_field_of_view, const float near_plane = 0.1f, const float far_plane = 100.0f );

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Camera );

	/* Matrix Getters: */

		const Matrix4x4& GetViewMatrix();
		const Matrix4x4& GetProjectionMatrix();
		const Matrix4x4& GetViewProjectionMatrix();

	/* View related: */

		const Vector3&		Scale()		const { return transform->GetScaling(); }
		const Vector3&		Position()	const { return transform->GetTranslation(); }
		const Quaternion&	Rotation()	const { return transform->GetRotation(); }

		Vector3 Right();
		Vector3 Up();
		Vector3 Forward();

		Camera& SetLookRotation( const Vector3& look_at, const Vector3& up = Vector3::Up() );

	/* Projection Related related: */

		Camera& SetNearPlaneOffset( const float offset );
		Camera& SetFarPlaneOffset( const float offset );
		float GetNearPlaneOffset() const { ASSERT_DEBUG_ONLY( UsesPerspectiveProjection() ); return plane_near; }
		float GetFarPlaneOffset()  const { ASSERT_DEBUG_ONLY( UsesPerspectiveProjection() ); return plane_far; }

		const float GetAspectRatio() const { ASSERT_DEBUG_ONLY( UsesPerspectiveProjection() ); return aspect_ratio; }
		Camera& SetAspectRatio( const float new_aspect_ratio );

		const Radians& GetVerticalFieldOfView() const { ASSERT_DEBUG_ONLY( UsesPerspectiveProjection() ); return vertical_field_of_view; }
		Camera& SetVerticalFieldOfView( const Radians new_fov );

		/* Using this will set an "overridden" flag.
		 * The custom projection matrix set within this function will be used instead of the perspective projection defined internally, until that flag is cleared:
		 *		A) Directly, by calling ClearCustomProjectionMatrix(),
		 *		B) Indirectly, by setting near/far planes, aspect ratio or vertical FoV. */
		Camera& SetCustomProjectionMatrix( const Matrix4x4& custom_projection_matrix );
		Camera& ClearCustomProjectionMatrix();
		bool UsesPerspectiveProjection() const { return projection_matrix.IsIdentity() || Matrix::IsPerspectiveProjection( projection_matrix ); }

	/* Other:*/

		Vector3 ConvertFromScreenSpaceToViewSpace( const Engine::Vector2 screen_space_coordinate, const Engine::Vector2I screen_dimensions );

	private:
		void SetProjectionMatrixDirty();
		void SetViewProjectionMatrixDirty();
		void SetCustomProjectionMatrixDirty();

	private:
		Matrix4x4 view_matrix;
		Matrix4x4 projection_matrix;
		Matrix4x4 view_projection_matrix;

		Transform* transform;

		float plane_near;
		float plane_far;
		float aspect_ratio;
		Radians vertical_field_of_view;

		bool projection_matrix_needs_update;
		bool view_projection_matrix_needs_update; // This is needed because access to View/Projection matrices causes their dirty flag to be reset.
		/* Do not need a "view_matrix_needs_update" flag; Transform's data is the only data needed to update the view matrix and it has the IsDirty() query already. */

		// TODO: The comment above is not correct:
		/* Transform's dirty flag is opt-in.
		 * Besides, Renderer can not use that flag to optimize Camera UBO uploads, since there may be multiple updates to the Camera transform in a frame.
		 */
		// TODO: Given these limitations of the Transform's dirty flag. Figure out how to tackle that in a more robust way.

		/* [6 bytes of padding remaining.] */
	};
}
