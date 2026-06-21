#pragma once

// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"
#include "Core/Macros.h"

namespace Kakadu
{
	class Transform
	{
	public:
		enum class Mask : u8
		{
			Scale       = 1,
			Translation = 2,
			Rotation    = 4,

			NoRotation    = Scale		| Translation,
			NoTranslation = Scale		| Rotation,
			NoScale       = Translation | Rotation,

			All = Scale | Translation | Rotation
		};

	public:
		Transform();
		Transform( const Vector3& scale );
		Transform( const Vector3& scale, const Vector3& translation );
		Transform( const Vector3& scale, const Quaternion& rotation, const Vector3& translation );

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Transform );

		~Transform();

	/* Modification: */

		Transform& Reset();

		Transform& ResetScaling();
		Transform& ResetRotation();
		Transform& ResetTranslation();

		Transform& SetScaling( const float& new_uniform_scale );
		Transform& SetScaling( const Vector3& new_scale );
		Transform& SetScaling( const float new_x_scale, const float new_y_scale, const float new_z_scale );
		Transform& OffsetScaling( const Vector3& delta );
		Transform& OffsetScaling( const float delta_x_scale, const float delta_y_scale, const float delta_z_scale );
		Transform& MultiplyScaling( const Vector3& multiplier );
		Transform& MultiplyScaling( const float multiplier );

		Transform& SetRotation( const Quaternion& new_rotation );
		Transform& SetRotation( const Radians heading_around_y, const Radians pitch_around_x, const Radians bank_around_z );
		Transform& SetRotation( const Degrees heading_around_y, const Degrees pitch_around_x, const Degrees bank_around_z );
		Transform& MultiplyRotation( const Quaternion& multiplier );
		Transform& MultiplyRotation_X( const Radians& angle );
		Transform& MultiplyRotation_Y( const Radians& angle );
		Transform& MultiplyRotation_Z( const Radians& angle );

		Transform& SetTranslation( const Vector3& new_translation );
		Transform& SetTranslation( const float new_x, const float new_y, const float new_z );
		Transform& OffsetTranslation( const Vector3& delta );
		Transform& OffsetTranslation( const float delta_x, const float delta_y, const float delta_z );
		Transform& MultiplyTranslation( const Vector3& multiplier );
		Transform& MultiplyTranslation( const float multiplier );

		Transform& LookAt( const Vector3& direction, const Vector3& up = Vector3::Up() );

		/* SRT = Scale * Rotate * Translate. */
		Transform& SetFromSRTMatrix( const Matrix4x4& srt_matrix );

	/* Queries: */

		const Vector3& GetScaling() const;
		const Quaternion& GetRotation() const;
		const Vector3& GetTranslation() const;

		const Matrix4x4& GetFinalMatrix();
		const Matrix4x4 GetInverseOfFinalMatrix();
		/* If the caller knows there's no scaling involved (for example; Transform of a Camera), calling this function is more preferrable, as it is cheaper to execute. */
		const Matrix4x4 GetInverseOfFinalMatrix_NoScale();

		const Vector3& Right();
		const Vector3& Up();
		const Vector3& Forward();

	private:
		void UpdateScalingMatrixIfDirty();
		void UpdateRotationPartOfMatrixIfDirty();
		void UpdateTranslationPartOfMatrixIfDirty();
		void UpdateFinalMatrixIfDirty();

		const Matrix4x4& GetScalingMatrix();
		const Matrix4x4& GetRotationAndTranslationMatrix();

	private:
		Vector3 scale;
		Vector3 translation;
		Quaternion rotation;

		Matrix4x4 scaling_matrix;
		Matrix4x4 rotation_and_translation_matrix;

		Matrix4x4 final_matrix;

		/* 4 flags below are for internal (Transform) use. */
		bool scaling_needsUpdate;
		bool rotation_needsUpdate;
		bool translation_needsUpdate;

		bool final_matrix_needsUpdate;
	};
}
