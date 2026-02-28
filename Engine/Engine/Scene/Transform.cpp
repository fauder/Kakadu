// Engine Includes.
#include "Math/Math.hpp"
#include "Math/Matrix.h"

#include "Transform.h"

namespace Kakadu
{
	Transform::Transform()
		:
		scale( Vector3( 1.0f, 1.0f, 1.0f ) ),
		translation( ZERO_INITIALIZATION ),
		rotation(),
		scaling_needsUpdate( true ),
		rotation_needsUpdate( true ),
		translation_needsUpdate( true ),
		final_matrix_needsUpdate( true ),
		is_dirty( false )
	{
	}

	Transform::Transform( const Vector3& scale )
		:
		scale( scale ),
		translation( ZERO_INITIALIZATION ),
		rotation(),
		scaling_needsUpdate( true ),
		rotation_needsUpdate( true ),
		translation_needsUpdate( true ),
		final_matrix_needsUpdate( true ),
		is_dirty( false )
	{
	}

	Transform::Transform( const Vector3& scale, const Vector3& translation )
		:
		scale( scale ),
		translation( translation ),
		rotation(),
		scaling_needsUpdate( true ),
		rotation_needsUpdate( true ),
		translation_needsUpdate( true ),
		final_matrix_needsUpdate( true ),
		is_dirty( false )
	{
	}

	Transform::Transform( const Vector3& scale, const Quaternion& rotation, const Vector3& translation )
		:
		scale( scale ),
		translation( translation ),
		rotation( rotation ),
		scaling_needsUpdate( true ),
		rotation_needsUpdate( true ),
		translation_needsUpdate( true ),
		final_matrix_needsUpdate( true ),
		is_dirty( false )
	{
	}

	Transform::~Transform()
	{
	}

	Transform& Transform::SetScaling( const float& new_uniform_scale )
	{
		return SetScaling( new_uniform_scale, new_uniform_scale, new_uniform_scale );
	}

	Transform& Transform::SetScaling( const Vector3& new_scale )
	{
		this->scale = new_scale;
		scaling_needsUpdate = final_matrix_needsUpdate = is_dirty = true;

		return *this;
	}

	Transform& Transform::SetScaling( const float new_x_scale, const float new_y_scale, const float new_z_scale )
	{
		this->scale.Set( new_x_scale, new_y_scale, new_z_scale );
		scaling_needsUpdate = final_matrix_needsUpdate = is_dirty = true;

		return *this;
	}

	Transform& Transform::OffsetScaling( const Vector3& delta )
	{
		return SetScaling( scale + delta );
	}

	Transform& Transform::OffsetScaling( const float delta_x_scale, const float delta_y_scale, const float delta_z_scale )
	{
		return SetScaling( scale.X() + delta_x_scale, scale.Y() + delta_y_scale, scale.Z() + delta_z_scale );
	}

	Transform& Transform::MultiplyScaling( const Vector3& multiplier )
	{
		return SetScaling( scale * multiplier );
	}

	Transform& Transform::MultiplyScaling( const float multiplier )
	{
		return SetScaling( scale * multiplier );
	}

	Transform& Transform::SetRotation( const Quaternion& new_rotation )
	{
		ASSERT_DEBUG_ONLY( new_rotation.IsNormalized() && R"(Transform::SetRotation(): The quaternion "new_rotation" is not normalized!)" );

		this->rotation = new_rotation;
		rotation_needsUpdate = final_matrix_needsUpdate = is_dirty = true;

		return *this;
	}

	Transform& Transform::SetRotation( const Radians heading_around_y, const Radians pitch_around_x, const Radians bank_around_z )
	{
		return SetRotation( Math::EulerToQuaternion( heading_around_y, pitch_around_x, bank_around_z ) );
	}

	Transform& Transform::SetRotation( const Degrees heading_around_y, const Degrees pitch_around_x, const Degrees bank_around_z )
	{
		return SetRotation( Math::EulerToQuaternion( heading_around_y, pitch_around_x, bank_around_z ) );
	}

	Transform& Transform::MultiplyRotation( const Quaternion& multiplier )
	{
		return SetRotation( ( rotation * multiplier ).Normalized() );
	}

	Transform& Transform::MultiplyRotation_X( const Radians& angle )
	{
		return SetRotation( ( rotation * Quaternion( angle, Vector3::Right() ) ).Normalized() );
	}

	Transform& Transform::MultiplyRotation_Y( const Radians& angle )
	{
		return SetRotation( ( rotation * Quaternion( angle, Vector3::Up() ) ).Normalized() );
	}

	Transform& Transform::MultiplyRotation_Z( const Radians& angle )
	{
		return SetRotation( ( rotation * Quaternion( angle, Vector3::Forward() ) ).Normalized() );
	}

	Transform& Transform::SetTranslation( const Vector3& new_translation )
	{
		this->translation = new_translation;
		translation_needsUpdate = final_matrix_needsUpdate = is_dirty = true;

		return *this;
	}

	Transform& Transform::SetTranslation( const float new_x, const float new_y, const float new_z )
	{
		this->translation.Set( new_x, new_y, new_z );
		translation_needsUpdate = final_matrix_needsUpdate = is_dirty = true;

		return *this;
	}

	Transform& Transform::OffsetTranslation( const Vector3& delta )
	{
		return SetTranslation( translation + delta );
	}

	Transform& Transform::OffsetTranslation( const float delta_x, const float delta_y, const float delta_z )
	{
		return SetTranslation( translation.X() + delta_x, translation.Y() + delta_y, translation.Z() + delta_z );
	}

	Transform& Transform::MultiplyTranslation( const Vector3& multiplier )
	{
		return SetTranslation( translation * multiplier );
	}

	Transform& Transform::MultiplyTranslation( const float multiplier )
	{
		return SetTranslation( translation * multiplier );
	}

	/* SRT = Scale * Rotate * Translate. */
	Transform& Transform::SetFromSRTMatrix( const Matrix4x4& srt_matrix )
	{
		Matrix::DecomposeSRT( srt_matrix, scale, rotation, translation );

		/* Test if the reconstructed matrix matches the input matrix: */
		ASSERT_DEBUG_ONLY( Matrix::SRT( scale, rotation, translation ) == srt_matrix )

		final_matrix_needsUpdate = scaling_needsUpdate = rotation_needsUpdate = translation_needsUpdate = is_dirty = true;

		return *this;
	}

	const Vector3& Transform::GetScaling() const
	{
		return scale;
	}

	const Quaternion& Transform::GetRotation() const
	{
		return rotation;
	}

	const Vector3& Transform::GetTranslation() const
	{
		return translation;
	}

	const Matrix4x4& Transform::GetFinalMatrix()
	{
		UpdateFinalMatrixIfDirty();

		return final_matrix;
	}

	const Matrix4x4 Transform::GetInverseOfFinalMatrix()
	{
		/* Instead of actually calculating the inverse of the matrix, we'll leverage the information we have on our two current matrices:
		 * Scaling Matrix: We can simply use the inverses (with respect to multiplication) of diagonal elements' values to reverse the scaling operation.
		 * Rotation And Translation Matrix:
		 *		We can simply transpose the rotation part (upper 3x3 portion of the 4x4 matrix) since rotation matrices are orthogonal.
		 *		We can simply use the inverse (with respect to addition) of the translation part (first 3 elements of the last row of the 4x4 matrix) to reverse the translation operation.
		*/

		// Force-update the matrices to get up-to-date values.
		GetScalingMatrix();
		GetRotationAndTranslationMatrix();

		const Vector4 inverse_scale( 1.0f / scaling_matrix[ 0 ][ 0 ], 1.0f / scaling_matrix[ 1 ][ 1 ], 1.0f / scaling_matrix[ 2 ][ 2 ], 1.0f );

		const Matrix4x4 inverse_scaling_matrix( inverse_scale );
		const Matrix4x4 inverse_rotation_matrix( rotation_and_translation_matrix.SubMatrix< 3 >().Transposed() );
		const Matrix4x4 inverse_translation_matrix( Matrix4x4{}.SetTranslation( -rotation_and_translation_matrix.GetRow< 3 >( 3 /* Last Row. */ ) ) );

		return inverse_translation_matrix * inverse_rotation_matrix * inverse_scaling_matrix;
	}

	/* If the caller knows there's no scaling involved (for example; Transform of a Camera), calling this function is more preferrable. */
	const Matrix4x4 Transform::GetInverseOfFinalMatrix_NoScale()
	{
		ASSERT_DEBUG_ONLY( scale == Vector3::One() );

		// Force-update the rotation part get up-to-date values.
		UpdateRotationPartOfMatrixIfDirty();

		/* Instead of actually calculating the inverse of the matrix, we'll leverage the information we have on our Rotation And Translation Matrix:
		 *		We can simply transpose the rotation part (upper 3x3 portion of the 4x4 matrix) since rotation matrices are orthogonal.
		 *		We can simply use the inverse of the translation to reverse the translation operation.
		 *
		 * Since the scaling is uniform (and in this case, 1), we can simply combine the inverse rotation (3x3 matrix) and the inverse translation (3D vector) in-place,
		 * to construct a 4x4 transformation matrix, instead of creating two separate matrices and multiplying them together, saving us some cycles.
		 *			BUT the inverse translation needs to be rotated by the inverse rotation first, to bring it to the correct space after the rotation is applied.
		 *			(Also, RT != (Ti)(Rt) (where i = inverse & t = transpose), so we can't just plug the Rt & Ti into a single matrix and call it a day).
		*/

		const Matrix3x3 inverse_rotation_matrix_3x3( rotation_and_translation_matrix.SubMatrix< 3 >().Transposed() );
		return Matrix4x4( inverse_rotation_matrix_3x3, -translation.XYZ() * inverse_rotation_matrix_3x3 );
	}

	const Vector3& Transform::Right()
	{
		UpdateRotationPartOfMatrixIfDirty();
		return rotation_and_translation_matrix.GetRow< 3 >( 0 );
	}

	const Vector3& Transform::Up()
	{
		UpdateRotationPartOfMatrixIfDirty();
		return rotation_and_translation_matrix.GetRow< 3 >( 1 );
	}

	const Vector3& Transform::Forward()
	{
		UpdateRotationPartOfMatrixIfDirty();
		return rotation_and_translation_matrix.GetRow< 3 >( 2 );
	}

	Transform& Transform::LookAt( const Vector3& direction, const Vector3& up )
	{
		return SetRotation( Quaternion::LookRotation( direction, up ) );
	}

	void Transform::UpdateScalingMatrixIfDirty()
	{
		if( scaling_needsUpdate )
		{
			scaling_matrix.SetDiagonals( scale );
			scaling_needsUpdate = false;
		}
	}

	void Transform::UpdateRotationPartOfMatrixIfDirty()
	{
		if( rotation_needsUpdate )
		{
			rotation_and_translation_matrix = Matrix4x4( Math::QuaternionToMatrix3x3( rotation ), rotation_and_translation_matrix.GetRow< 3 >( 3 ) );
			rotation_needsUpdate = false;
		}
	}

	void Transform::UpdateTranslationPartOfMatrixIfDirty()
	{
		if( translation_needsUpdate )
		{
			rotation_and_translation_matrix.SetTranslation( translation );
			translation_needsUpdate = false;
		}
	}

	void Transform::UpdateFinalMatrixIfDirty()
	{
		if( final_matrix_needsUpdate )
		{
			final_matrix = GetScalingMatrix() * GetRotationAndTranslationMatrix();
			final_matrix_needsUpdate = false;
		}
	}

	const Matrix4x4& Transform::GetScalingMatrix()
	{
		UpdateScalingMatrixIfDirty();

		return scaling_matrix;
	}

	const Matrix4x4& Transform::GetRotationAndTranslationMatrix()
	{
		UpdateRotationPartOfMatrixIfDirty();
		UpdateTranslationPartOfMatrixIfDirty();

		return rotation_and_translation_matrix;
	}
}
