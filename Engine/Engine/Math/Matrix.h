#pragma once

// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"
#include "Math/Quaternion.hpp"

namespace Engine::Matrix
{
	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	constexpr Matrix4x4 OrthographicProjection( const float left_plane_offset, const float right_plane_offset, const float bottom_plane_offset, const float top_plane_offset, 
												const float near_plane_offset, const float far_plane_offset )
	{
		const float f_plus_n  = far_plane_offset + near_plane_offset;
		const float f_minus_n = far_plane_offset - near_plane_offset;

		return Matrix4x4
		(
			{
				1.0f / right_plane_offset,			0.0f,								0.0f,						0.0f,
				0.0f,								1.0f / top_plane_offset,			0.0f,						0.0f,
				0.0f,								0.0f,								2.0f / f_minus_n,			0.0f,
				0.0f,								0.0f,								-f_plus_n / f_minus_n,		1.0f
			}
		);
	}

	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	Matrix4x4 PerspectiveProjection( const float near_plane_offset, const float far_plane_offset,
									 const float aspect_ratio, const Radians vertical_field_of_view );

	constexpr bool IsPerspectiveProjection( const Matrix4x4 matrix )
	{
		return Math::IsEqual( matrix[ 2 ][ 3 ], 1.0f );
	}

	/* In row-major form. */ constexpr Matrix4x4 Scaling( const float new_scale )
	{
		return Matrix4x4
		(
			{
				new_scale,	0.0f,		0.0f,		0.0f,
				0.0f,		new_scale,	0.0f,		0.0f,
				0.0f,		0.0f,		new_scale,	0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Scaling( const float new_scale_x, const float new_scale_y, const float new_scale_z )
	{
		return Matrix4x4
		(
			{
				new_scale_x,	0.0f,			0.0f,			0.0f,
				0.0f,			new_scale_y,	0.0f,			0.0f,
				0.0f,			0.0f,			new_scale_z,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Scaling( const Vector3& scale_vector )
	{
		return Matrix4x4
		(
			{
				scale_vector.X(),	0.0f,				0.0f,				0.0f,
				0.0f,				scale_vector.Y(),	0.0f,				0.0f,
				0.0f,				0.0f,				scale_vector.Z(),	0.0f,
				0.0f,				0.0f,				0.0f,				1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnX( const float new_scale )
	{
		return Matrix4x4
		(
			{
				new_scale,	0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnY( const float new_scale )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		new_scale,	0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 ScalingOnZ( const float new_scale )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		new_scale,	0.0f,
				0.0f,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundX( Radians pitch );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundY( Radians heading );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundZ( Radians bank );
	/* In row-major form. Counter-clockwise rotation. */ Matrix4x4 RotationAroundAxis( Radians angle, Vector3 vector );
	/* In row-major form. Counter-clockwise rotation. */ void RotationAroundAxis( Matrix4x4& matrix, Radians angle, Vector3 vector );

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnX( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta,		0.0f,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnY( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		delta,		0.0f,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 TranslationOnZ( const float delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				0.0f,		0.0f,		delta,		1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Translation( const float delta_x, const float delta_y, const float delta_z )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta_x,	delta_y,	delta_z,	1.0f
			}
		);
	}

	/* In row-major form. */ constexpr Matrix4x4 Translation( const Vector3 delta )
	{
		return Matrix4x4
		(
			{
				1.0f,		0.0f,		0.0f,		0.0f,
				0.0f,		1.0f,		0.0f,		0.0f,
				0.0f,		0.0f,		1.0f,		0.0f,
				delta.X(),	delta.Y(),	delta.Z(),	1.0f
			}
		);
	}

	/* In row-major form. Left-handed. */
	constexpr Matrix4x4 LookAt( const Vector3& camera_position, Vector3 to_target_normalized, const Vector3& world_up_normalized = Vector3::Up() )
	{
		ASSERT_DEBUG_ONLY( to_target_normalized.IsNormalized() && R"(Matrix::LookAt(): "to_target_normalized" is not normalized!)" );
		ASSERT_DEBUG_ONLY(  world_up_normalized.IsNormalized() && R"(Matrix::LookAt():  "world_up_normalized" is not normalized!)" );

		const auto to_camera_right_normalized = Math::Cross( world_up_normalized, to_target_normalized );
		const auto to_camera_up_normalized    = Math::Cross( to_target_normalized, to_camera_right_normalized );

		return Matrix::Translation( -camera_position ) * Matrix4x4
		{
			{ // This is the TRANSPOSED rotation matrix, utilizing the basis constructed above.
				to_camera_right_normalized.X(),		to_camera_up_normalized.X(),	to_target_normalized.X(),	0.0f,
				to_camera_right_normalized.Y(),		to_camera_up_normalized.Y(),	to_target_normalized.Y(),	0.0f,
				to_camera_right_normalized.Z(),		to_camera_up_normalized.Z(),	to_target_normalized.Z(),	0.0f,
				0.0f,								0.0f,							0.0f,						1.0f
			}
		};
	}

	/* SRT = Scale * Rotate * Translate. */
	Matrix4x4 SRT( const Vector3& scale, const Quaternion& rotation, const Vector3& translation );
	/* SRT = Scale * Rotate * Translate. */
	void SRT( Matrix4x4& srt_matrix, const Vector3& scale, const Quaternion& rotation, const Vector3& translation );
	/* SRT = Scale * Rotate * Translate. */
	void DecomposeSRT( const Matrix4x4& srt_matrix, Vector3& scale, Quaternion& rotation, Vector3& translation );

	Vector3 CameraWorldPositionFromViewMatrix( const Matrix4x4& view_matrix );
};
