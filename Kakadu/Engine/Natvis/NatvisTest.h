#pragma once

// Engine Includes.
#include "Core/Macros.h"
#include "Math/Constants.h"
#include "Math/Matrix.h" // Includes Angle.hpp, Matrix.hpp, Quaternion.hpp, Vector.hpp.

namespace Natvis
{
	/* Exercise every display case in Engine.natvis.
	 * Set a breakpoint at the first declaration, then step over each line and inspect
	 * the variable in the watch/locals window.
	 * 
	 * Texture & Framebuffer cases require live GL objects and
	 * must be verified through normal application use. */
	header_function void Test()
	{
		using namespace Kakadu;

		/* Vector2< float >: */
		const Vector2 v2f_not_normalized{ 3.0f, 4.0f }; // [x]: 3 [y]: 4.
		const Vector2 v2f_normalized{ 1.0f, 0.0f }; // [x]: 1 [y]: 0 (normalized).

		/* Vector2< double >: */
		const Vector2D v2d_not_normalized{ 3.0, 4.0 };
		const Vector2D v2d_normalized{ 1.0, 0.0 }; // (normalized).

		/* Vector2< int >: */
		const Vector2I v2i{ 1, 2 };

		/* Vector3< float >: */
		const Vector3 v3f_not_normalized{ 1.0f, 2.0f, 3.0f };
		const Vector3 v3f_normalized{ 1.0f, 0.0f, 0.0f }; // (normalized).

		/* Vector3< double >: */
		const Vector3D v3d_not_normalized{ 1.0, 2.0, 3.0 };
		const Vector3D v3d_normalized{ 1.0, 0.0, 0.0 }; // (normalized).

		/* Vector3< int >: */
		const Vector3I v3i{ 1, 2, 3 };

		/* Vector4< float >: */
		const Vector4 v4f_not_normalized{ 1.0f, 2.0f, 3.0f, 4.0f };
		const Vector4 v4f_normalized{ 0.0f, 0.0f, 0.0f, 1.0f }; // (normalized).

		/* Vector4< double >: */
		const Vector4D v4d_not_normalized{ 1.0, 2.0, 3.0, 4.0 };
		const Vector4D v4d_normalized{ 0.0, 0.0, 0.0, 1.0 }; // (normalized).

		/* Vector4< int >: */
		const Vector4I v4i{ 1, 2, 3, 4 };

		/* Degrees< float >: */
		const Math::Degrees< float > deg_f{ 90.0f }; // 90 degrees (1.5708 radians).

		/* Degrees< double >: */
		const Math::Degrees< double > deg_d{ 90.0 };

		/* Radians< float >: */
		const Math::Radians< float > rad_f{ Constants< float >::Pi_Over_Two() }; // 1.5708 radians (90 degrees).

		/* Radians< double >: */
		const Math::Radians< double > rad_d{ Constants< double >::Pi_Over_Two() };

		/* Quaternion< float >: */
		const Quaternion q_f_identity{}; // Identity Rotation.
		const Quaternion q_f_rot_z_90 = Quaternion::RotateAroundZ_By_PiOverTwo(); // Rotation Angle: 90 degrees | Axis: <0,0,1>.
		const Quaternion q_f_unnormalized( 1.0f, 1.0f, 1.0f, 1.0f ); // UNNORMALIZED->NOT A ROTATION.

		/* Quaternion< double >: */
		const QuaternionD q_d_identity{};
		const QuaternionD q_d_rot_z_90 = QuaternionD::RotateAroundZ_By_PiOverTwo();
		const QuaternionD q_d_unnormalized( 1.0, 1.0, 1.0, 1.0 );

		/* Matrix< float, 3, 3 >: */

		constexpr float s = 0.70710678f; // sin(45 degrees) = cos(45 degrees).

		/* IsIdentity: */
		const Matrix3x3 m33_identity{};

		/* IsXRotationOnly: 45 degrees around X (row-major, v'=v*Rx):
		 * Row 0: [1,   0,  0 ]
		 * Row 1: [0,  cos, sin]
		 * Row 2: [0, -sin, cos] */
		const Matrix3x3 m33_x_rot_45( Vector3{ 1.0f, 0.0f, 0.0f }, Vector3{ 0.0f, s, s }, Vector3{ 0.0f, -s, s } );

		/* IsYRotationOnly: 45 degrees around Y:
		 * Row 0: [cos, 0, -sin]
		 * Row 1: [0,   1,   0 ]
		 * Row 2: [sin, 0,  cos] */
		const Matrix3x3 m33_y_rot_45( Vector3{ s, 0.0f, -s }, Vector3{ 0.0f, 1.0f, 0.0f }, Vector3{ s, 0.0f, s } );

		/* IsZRotationOnly: 45 degrees around Z:
		 * Row 0: [ cos, sin, 0]
		 * Row 1: [-sin, cos, 0]
		 * Row 2: [  0,   0,  1] */
		const Matrix3x3 m33_z_rot_45( Vector3{ s, s, 0.0f }, Vector3{ -s, s, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f } );

		/* IsUniformScale: Diag(2,2,1): row 0 has length 2, excluded from IsZRotationOnly by the unit-row check. */
		const Matrix3x3 m33_uniform_scale( Vector3{ 2.0f, 2.0f, 1.0f } ); // Scales by 2 | Matrix3x3.

		/* IsNonUniformScale: Diag(2,3,1): data[0][0]!=data[1][1] prevents IsZRotationOnly's IsEqual(0,1). */
		const Matrix3x3 m33_non_uniform_scale( Vector3{ 2.0f, 3.0f, 1.0f } ); // Scales by x: 2 | y: 3 | Matrix3x3.

		/* IsDiagonalOnly: Diag(2,3,2): data[2][2]=2!=1 prevents IsScale, so no rotation/scale condition matches. */
		const Matrix3x3 m33_diagonal_only( Vector3{ 2.0f, 3.0f, 2.0f } ); // Diagonal (2,3,2) | Matrix3x3.

		/* IsOrthonormal: 120 degrees around (1,1,1)/sqrt(3): cyclic permutation (x->y, y->z, z->x).
		 * None of the axis-aligned checks apply, so falls through to the generic orthonormal branch. */
		const Matrix3x3 m33_generic_rot( Vector3{ 0.0f, 1.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 1.0f }, Vector3{ 1.0f, 0.0f, 0.0f } );

		/* Matrix< float, 4, 4 >: */

		/* IsIdentity: */
		const Matrix4x4 m44_identity{};

		/* IsTranslationOnly: Identity 3x3 block, nonzero row-3 translation: */
		Matrix4x4 m44_translation_only{};
		m44_translation_only[ 3 ][ 0 ] = 1.0f;
		m44_translation_only[ 3 ][ 1 ] = 2.0f;
		m44_translation_only[ 3 ][ 2 ] = 3.0f; // Translates by x: 1 | y: 2 | z: 3 | Matrix4x4.

		/* IsXRotationOnly: 45 degrees around X (built from 3x3): */
		const Matrix4x4 m44_x_rot_45( m33_x_rot_45 );

		/* IsYRotationOnly: 45 degrees around Y: */
		const Matrix4x4 m44_y_rot_45( m33_y_rot_45 );

		/* IsZRotationOnly: 45 degrees around Z: */
		const Matrix4x4 m44_z_rot_45( m33_z_rot_45 );

		/* IsNonUniformScale: Diag(1,2,2,1): row 1 has length 2, excluded from IsXRotationOnly by the unit-row check. */
		const Matrix4x4 m44_yz_scale( Vector4{ 1.0f, 2.0f, 2.0f, 1.0f } ); // Scales by x: 1 | y: 2 | z: 2 | Matrix4x4.

		/* IsNonUniformScale: Diag(2,1,2,1): row 0 has length 2, excluded from IsYRotationOnly by the unit-row check. */
		const Matrix4x4 m44_xz_scale( Vector4{ 2.0f, 1.0f, 2.0f, 1.0f } ); // Scales by x: 2 | y: 1 | z: 2 | Matrix4x4.

		/* IsNonUniformScale: Diag(2,2,1,1): row 0 has length 2, excluded from IsZRotationOnly by the unit-row check. */
		const Matrix4x4 m44_xy_scale( Vector4{ 2.0f, 2.0f, 1.0f, 1.0f } ); // Scales by x: 2 | y: 2 | z: 1 | Matrix4x4.

		/* IsUniformScale: Diag(2,2,2,1): Is1(2,2) is false (data[2][2]=2), so IsZRotationOnly does not fire. */
		const Matrix4x4 m44_uniform_scale( Vector4{ 2.0f, 2.0f, 2.0f, 1.0f } ); // Scales by 2 | Matrix4x4.

		/* IsNonUniformScale: Diag(2,3,4,1): */
		const Matrix4x4 m44_non_uniform_scale( Vector4{ 2.0f, 3.0f, 4.0f, 1.0f } ); // Scales by x: 2 | y: 3 | z: 4 | Matrix4x4.

		/* IsDiagonalOnly: Diag(2,3,4,2): data[3][3]=2!=1 prevents IsScale, so none of the named
		 * scale/rotation conditions apply. */
		const Matrix4x4 m44_diagonal_only( Vector4{ 2.0f, 3.0f, 4.0f, 2.0f } ); // Diagonal (2,3,4,2) | Matrix4x4.

		/* IsOrthonormal (no translation): Same cyclic permutation as m33_generic_rot: */
		const Matrix4x4 m44_generic_rot( m33_generic_rot ); // Rotates by 120 degrees around <0.577,0.577,0.577> | Matrix4x4.

		/* IsOrthonormal & HasTranslation: Same rotation block plus nonzero translation row: */
		Matrix4x4 m44_generic_rot_with_translation( m33_generic_rot );
		m44_generic_rot_with_translation[ 3 ][ 0 ] = 1.0f;
		m44_generic_rot_with_translation[ 3 ][ 1 ] = 2.0f;
		m44_generic_rot_with_translation[ 3 ][ 2 ] = 3.0f; // Rotates by 120 degrees around <...> + Translates by x: 1 | y: 2 | z: 3 | Matrix4x4.

		/* No matching display: Affine matrix with shear (not diagonal, not orthonormal).
		 * Falls back to the Expand section: rows are visible but no DisplayString fires. */
		const Matrix4x4 m44_no_match( Vector4{ 2.0f, 1.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 2.0f, 0.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 2.0f, 0.0f }, Vector4{ 0.0f, 0.0f, 0.0f, 1.0f } );

		/* Generic Matrix< T, R, C > wildcard visualizer: Triggers the catch-all <Type Name="Kakadu::Math::Matrix<*>"> entry. */
		const Math::Matrix< float, 2, 3 > m_2x3{};

		const char* dummy = "Breakpoint here";
	}
}
