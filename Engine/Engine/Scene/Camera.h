#pragma once

// Engine Includes.
#include "Transform.h"
#include "Math/Matrix.h"

namespace Kakadu
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

		Vector3 ConvertFromScreenSpaceToViewSpace( const Kakadu::Vector2 screen_space_coordinate, const Kakadu::Vector2I screen_dimensions );

	private:
		void SetProjectionMatrixDirty();
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

		/* There are intentionally no view_matrix_needs_update / view_projection_matrix_needs_update flags:
		 * GetViewMatrix() and GetViewProjectionMatrix() always recompute. The only way to know the view had changed used to be
		 * Transform's is_dirty, which was an artefact => removed. With no way left to detect a transform change, a
		 * projection-only dirty flag would leave the view-projection matrix stale whenever the camera moves, so it can not
		 * be cached safely. projection_matrix_needs_update is kept because the projection changes only through explicit
		 * setters, which it fully captures. To bring view-projection caching back in the future, a monotonic version
		 * counter (bumped on mutation) could be added to Transform and the observes can cache the last-seen version and check that. */
		bool projection_matrix_needs_update;
		/* 7 bytes of padding. */
	};
}
