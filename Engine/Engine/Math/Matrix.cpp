// Engine Includes.
#include "Math/Angle.hpp"
#include "Math/Math.hpp"
#include "Math/Matrix.h"

namespace Engine::Matrix
{
	/* In row-major form. Coordinates are in a left-handed coordinate system (both before & after multiplication). */
	Matrix4x4 PerspectiveProjection( const float near_plane_offset, const float far_plane_offset, const float aspect_ratio, const Radians vertical_field_of_view )
	{
		const Radians half_v_fov( vertical_field_of_view / 2.0f );
		const float half_tangent = Math::Tan( half_v_fov );

		const float f_plus_n    = far_plane_offset + near_plane_offset;
		const float f_minus_n   = far_plane_offset - near_plane_offset;
		const float f_n			= far_plane_offset * near_plane_offset;

		return Matrix4x4
		(
			{
				1.0f / ( aspect_ratio * half_tangent ),		0.0f,					0.0f,							0.0f,
				0.0f,										1.0f / half_tangent,	0.0f,							0.0f,
				0.0f,										0.0f,					f_plus_n / f_minus_n,			1.0f,
				0.0f,										0.0f,					-2.0f * f_n / f_minus_n,		0.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundX( Radians pitch )
	{
		const auto cosine_term = Math::Cos( pitch );
		const auto   sine_term = Math::Sin( pitch );

		return Matrix4x4
		(
			{
				1.0f,			0.0f,			0.0f,			0.0f,
				0.0f,			cosine_term,	sine_term,		0.0f,
				0.0f,			-sine_term,		cosine_term,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundY( Radians heading )
	{
		const auto cosine_term = Math::Cos( heading );
		const auto   sine_term = Math::Sin( heading );

		return Matrix4x4
		(
			{
				cosine_term,	0.0f,			-sine_term,		0.0f,
				0.0f,			1.0f,			0.0f,			0.0f,
				sine_term,		0.0f,			cosine_term,	0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundZ( Radians bank )
	{
		const auto cosine_term = Math::Cos( bank );
		const auto   sine_term = Math::Sin( bank );

		return Matrix4x4
		(
			{
				cosine_term,	sine_term,		0.0f,			0.0f,
				-sine_term,		cosine_term,	0.0f,			0.0f,
				0.0f,			0.0f,			1.0f,			0.0f,
				0.0f,			0.0f,			0.0f,			1.0f
			}
		);
	}

	/* In row-major form. Counter-clockwise rotation. */
	Matrix4x4 RotationAroundAxis( Radians angle, Vector3 vector )
	{
		vector.Normalize();

		const Radians theta( angle );

		const auto nx                           = vector.X();
		const auto ny                           = vector.Y();
		const auto nz                           = vector.Z();
		const auto cos_theta                    = Math::Cos( theta );
		const auto sin_theta                    = Math::Sin( theta );
		const auto one_minus_cos_theta          = 1.0f - cos_theta;
		const auto nx_times_one_minus_cos_theta = nx * one_minus_cos_theta;
		const auto ny_times_one_minus_cos_theta = ny * one_minus_cos_theta;
		const auto nz_times_one_minus_cos_theta = nz * one_minus_cos_theta;
		const auto nx_sin_theta                 = nx * sin_theta;
		const auto ny_sin_theta                 = ny * sin_theta;
		const auto nz_sin_theta                 = nz * sin_theta;

		return Matrix4x4
		(
			{
				nx * nx_times_one_minus_cos_theta + cos_theta,		ny * nx_times_one_minus_cos_theta + nz_sin_theta,		nz * nx_times_one_minus_cos_theta - ny_sin_theta,	0.0f,
				nx * ny_times_one_minus_cos_theta - nz_sin_theta,	ny * ny_times_one_minus_cos_theta + cos_theta,			nz * ny_times_one_minus_cos_theta + nx_sin_theta,	0.0f,
				nx * nz_times_one_minus_cos_theta + ny_sin_theta,	ny * nz_times_one_minus_cos_theta - nx_sin_theta,		nz * nz_times_one_minus_cos_theta + cos_theta,		0.0f,
				0.0f,												0.0f,													0.0f,												1.0f
			}
		);
	}

	/* In-place modification of the upper-left 3x3 portion. */
	/* In row-major form. Counter-clockwise rotation. */
	void RotationAroundAxis( Matrix4x4& matrix, Radians angle, Vector3 vector )
	{
		vector.Normalize();

		const Radians theta( angle );

		const auto nx                           = vector.X();
		const auto ny                           = vector.Y();
		const auto nz                           = vector.Z();
		const auto cos_theta                    = Math::Cos( theta );
		const auto sin_theta                    = Math::Sin( theta );
		const auto one_minus_cos_theta          = 1.0f - cos_theta;
		const auto nx_times_one_minus_cos_theta = nx * one_minus_cos_theta;
		const auto ny_times_one_minus_cos_theta = ny * one_minus_cos_theta;
		const auto nz_times_one_minus_cos_theta = nz * one_minus_cos_theta;
		const auto nx_sin_theta                 = nx * sin_theta;
		const auto ny_sin_theta                 = ny * sin_theta;
		const auto nz_sin_theta                 = nz * sin_theta;

		matrix[ 0 ][ 0 ] = nx * nx_times_one_minus_cos_theta + cos_theta;		matrix[ 0 ][ 1 ] = ny * nx_times_one_minus_cos_theta + nz_sin_theta;	matrix[ 0 ][ 2 ] = nz * nx_times_one_minus_cos_theta - ny_sin_theta;
		matrix[ 1 ][ 0 ] = nx * ny_times_one_minus_cos_theta - nz_sin_theta;	matrix[ 1 ][ 1 ] = ny * ny_times_one_minus_cos_theta + cos_theta;		matrix[ 1 ][ 2 ] = nz * ny_times_one_minus_cos_theta + nx_sin_theta;
		matrix[ 2 ][ 0 ] = nx * nz_times_one_minus_cos_theta + ny_sin_theta;	matrix[ 2 ][ 1 ] = ny * nz_times_one_minus_cos_theta - nx_sin_theta;	matrix[ 2 ][ 2 ] = nz * nz_times_one_minus_cos_theta + cos_theta;	
	}

	/* SRT = Scale * Rotate * Translate. */
	Matrix4x4 SRT( const Vector3& scale, const Quaternion& rotation, const Vector3& translation )
	{
#ifdef _DEBUG
		Matrix4x4 result( Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) );
		result.SetRow< 3 >( translation, 3 );

		ASSERT( result == ( Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) * Matrix::Translation( translation ) ) );
		
		return result;
#else
		return ( Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) ).SetRow< 3 >( translation, 3 );
#endif // _DEBUG
	}

	/* SRT = Scale * Rotate * Translate. */
	void SRT( Matrix4x4& srt_matrix, const Vector3& scale, const Quaternion& rotation, const Vector3& translation )
	{
		srt_matrix = ( Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) ).SetRow< 3 >( translation, 3 );

		ASSERT_DEBUG_ONLY( srt_matrix == ( Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) * Matrix::Translation( translation ) ) );
	}

	/* SRT = Scale * Rotate * Translate. */
	void DecomposeSRT( const Matrix4x4& srt_matrix, Vector3& scale, Quaternion& rotation, Vector3& translation )
	{
		translation = srt_matrix.GetRow< 3 >( 3, 0 );

		const Vector3& row_0 = srt_matrix.GetRow< 3 >( 0 );
		const Vector3& row_1 = srt_matrix.GetRow< 3 >( 1 );
		const Vector3& row_2 = srt_matrix.GetRow< 3 >( 2 );

		scale = Vector3( row_0.Magnitude(), row_1.Magnitude(), row_2.Magnitude() );

		Matrix3x3 rotation_matrix( ( row_0 / scale.X() ).Normalized(), ( row_1 / scale.Y() ).Normalized(), ( row_2 / scale.Z() ).Normalized() );

		/*if( not Math::IsEqual( scale.X(), scale.Y() ) || not Math::IsEqual( scale.Y(), scale.Z() ) )
		{
			// TODO: How to handle non-uniform scale here?
		}*/

		if( rotation_matrix.Determinant() < 0.0f ) // "Weed out" the reflection.
		{
			scale[ 0 ]                *= -1.0f;
			rotation_matrix[ 0 ][ 0 ] *= -1.0f;
		}

		rotation = Math::MatrixToQuaternion( rotation_matrix ).Normalized();
	}

	Vector3 CameraWorldPositionFromViewMatrix( const Matrix4x4& view_matrix )
	{
		const Matrix3x3 inverse_view_rotation( view_matrix.SubMatrix< 3 >().Transposed() );
		const Vector3 inverse_view_translation( -view_matrix.GetRow< 3 >( 3 ) );

		return inverse_view_rotation * inverse_view_translation;
	}
}