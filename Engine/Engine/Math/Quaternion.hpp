#pragma once

// Engine Includes.
#include "Core/Assertion.h"
#include "Angle.hpp"
#include "Concepts.h"
#include "Math.hpp"
#include "TypeTraits.h"
#include "Vector.hpp"

namespace Engine::Math
{
/* Forward Declarations. */
	template< std::floating_point ComponentType >
	struct Quaternion;

	template< std::floating_point ComponentType >
	constexpr ComponentType Dot( const Quaternion< ComponentType >& q1, const Quaternion< ComponentType >& q2 );

	template< Concepts::Arithmetic_NotBool Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize > && Concepts::NonZero< ColumnSize >
	struct Matrix;

	template< std::floating_point ComponentType >
	struct Quaternion
	{
		using VectorType  = Math::Vector< ComponentType, 3 >;
		using RadiansType = Math::Radians< ComponentType >;

	/* Constructors. */
		/* Identity quaternion w=1 & n=<0,0,0>. */
		constexpr Quaternion()
			:
			xyz( ZERO_INITIALIZATION ),
			w( 1 )
		{}

		CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Quaternion );

		constexpr ~Quaternion() = default;

		constexpr Quaternion( const ComponentType x, const ComponentType y, const ComponentType z, const ComponentType w )
			:
			xyz( x, y, z ),
			w( w )
		{}

		constexpr Quaternion( const VectorType& xyz, const ComponentType w )
			:
			xyz( xyz ),
			w( w )
		{}

		/* Promotes a 3D vector to a pure quaternion. */
		constexpr Quaternion( const VectorType& xyz )
			:
			xyz( xyz ),
			w( ComponentType( 0 ) )
		{}

		/* Expects a unit vector for the axis! */
		Quaternion( RadiansType angle, const VectorType& rotation_axis_normalized )
		{
			const RadiansType half_angle = angle * ComponentType( 0.5 );
			w   = Cos( half_angle );
			xyz = SinFromCos( w ) * rotation_axis_normalized;

			ASSERT_DEBUG_ONLY( rotation_axis_normalized.IsNormalized() && "QuaternionBase::QuaternionBase( angle, axis ): The axis vector provided is not normalized!" );
		}

	/* Comparison Operators. */
		bool operator==( const Quaternion & right_hand_side ) const
		{
			return Math::IsEqual( w, right_hand_side.w ) && xyz == right_hand_side.xyz;
		}

		bool operator!=( const Quaternion & right_hand_side ) const
		{
			return !operator==( right_hand_side );
		}

	/* Getters & Setters. */
		constexpr ComponentType X() const { return x; }
		constexpr ComponentType Y() const { return y; }
		constexpr ComponentType Z() const { return z; }
		constexpr ComponentType W() const { return w; }

	/* Other Queries. */
		static consteval Quaternion Identity() { return {}; }

		static consteval Quaternion RotateAroundX_By_Pi()			{ return Quaternion( Vector< ComponentType, 3 >::Right(),									ComponentType{ 0 } ); }
		static consteval Quaternion RotateAroundY_By_Pi()			{ return Quaternion( Vector< ComponentType, 3 >::Up(),										ComponentType{ 0 } ); }
		static consteval Quaternion RotateAroundZ_By_Pi()			{ return Quaternion( Vector< ComponentType, 3 >::Forward(),									ComponentType{ 0 } ); }
		static consteval Quaternion RotateAroundX_By_PiOverTwo()	{ return Quaternion( Vector< ComponentType, 3 >::Right()	* ComponentType{ 0.70710678 },	ComponentType{ 0.70710678 } ); }
		static consteval Quaternion RotateAroundY_By_PiOverTwo()	{ return Quaternion( Vector< ComponentType, 3 >::Up()		* ComponentType{ 0.70710678 },	ComponentType{ 0.70710678 } ); }
		static consteval Quaternion RotateAroundZ_By_PiOverTwo()	{ return Quaternion( Vector< ComponentType, 3 >::Forward()	* ComponentType{ 0.70710678 },	ComponentType{ 0.70710678 } ); }

		constexpr RadiansType HalfAngle() const
		{
			ASSERT_DEBUG_ONLY( IsNormalized() && R"(QuaternionBase::HalfAngle(): The quaternion "*this" is not normalized!)" );

			return RadiansType( Acos( w ) );
		}

		// Returns half the angular displacement between *this Quaternion and the other.
		constexpr RadiansType HalfAngleBetween( const Quaternion& other ) const
		{
			ASSERT_DEBUG_ONLY( this->IsNormalized() && R"(Quaternion::HalfAngleBetween(other) : The quaternion "*this" is not normalized!)" );
			ASSERT_DEBUG_ONLY( other.IsNormalized() && R"(Quaternion::HalfAngleBetween(other) : The quaternion "other" is not normalized!)" );

			return RadiansType( Acos( Math::Dot( *this, other ) ) );
		}

		constexpr RadiansType Angle() const
		{
			return ComponentType{ 2 } * HalfAngle();
		}

		// Returns the angular displacement between *this Quaternion and the other.
		constexpr RadiansType AngleBetween( const Quaternion& other ) const
		{
			return ComponentType{ 2 } * HalfAngleBetween( other );
		}

		/* Returns a NaN vector when the half angle represented by *this is zero. */
		constexpr VectorType Axis() const
		{
			ASSERT_DEBUG_ONLY( IsNormalized() && R"(QuaternionBase::Axis(): The quaternion "*this" is not normalized!)" );

			// Instead of incurring the cost of sin(HalfAngle()), sin(theta/2) can be derived by sqrt(1-w*w), as we know w = cos(theta/2).
			return VectorType( xyz / SinFromCos( w ) );
		}

		constexpr ComponentType MagnitudeSquared() const { return Dot(); }
		ComponentType Magnitude() const { return Sqrt( MagnitudeSquared() ); }

		constexpr bool IsIdentity() const
		{
			return Math::IsEqual( Abs( w ), ComponentType( 1 ) ) && xyz.IsZero();
		}

		constexpr bool IsNormalized() const
		{
			return Math::IsEqual( MagnitudeSquared(), ComponentType( 1 ) );
		}

	/* Arithmetic Operations: Unary operators. */
		constexpr Quaternion operator- () const
		{
			return *this * -1;
		}

	/* Arithmetic Operations: Binary operators (with a Quaternion). */
		constexpr Quaternion operator+ ( const Quaternion other ) const
		{
			return { xyz + other.xyz, w + other.w };
		}

		constexpr Quaternion& operator+= ( const Quaternion other )
		{
			xyz += other.xyz;
			w   += other.w;

			return *this;
		}

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form quaternion-operator-scalar. */
		constexpr Quaternion operator* ( const ComponentType scalar ) const
		{
			return { xyz * scalar, w * scalar };
		}

		constexpr Quaternion& operator*= ( const ComponentType scalar )
		{
			w   *= scalar;
			xyz *= scalar;

			return *this;
		}

		constexpr Quaternion operator/ ( const ComponentType scalar ) const
		{
			const auto inverse_of_scalar = ComponentType( 1 ) / scalar; // We can calculate the inverse safely.
			return { xyz * inverse_of_scalar, w * inverse_of_scalar };
		}

		constexpr Quaternion& operator/= ( const ComponentType scalar )
		{
			const auto inverse_of_scalar = ComponentType( 1 ) / scalar; // We can calculate the inverse safely.
			w *= inverse_of_scalar;
			xyz *= inverse_of_scalar;

			return *this;
		}

	/* Other Arithmetic Operations. */
		constexpr Quaternion operator* ( const Quaternion& other ) const
		{
			return Quaternion( w * other.xyz + other.w * xyz + Math::Cross( xyz, other.xyz ),
							   w * other.w - Math::Dot( xyz, other.xyz ) );
		}

		constexpr Quaternion operator*= ( const Quaternion& other )
		{
			return *this = Quaternion( *this ) * other;
		}

		/* Do not use this as it is slow.
		 * Use Transform(), as it is faster. */
		constexpr Vector3 Transform_Naive( const Vector3& vector_to_rotate ) const
		{
			return ( *this * Quaternion( vector_to_rotate ) * Inverse() ).xyz;
		}

		/* Use this as it is faster than Transform_Naive(). */
		constexpr Vector3 Transform( const Vector3& vector_to_rotate ) const
		{
			ASSERT_DEBUG_ONLY( IsNormalized() && R"(QuaternionBase::Transform(): The quaternion "*this" is not normalized!)" ); // The derivation assumes unit quaternions.

			/* Derivation can be found at https://gamesandsimulations.fandom.com/wiki/Quaternions, which is dead but can be retrieved via wayback machine at
			 * https://web.archive.org/web/20191115092410/https://gamesandsimulations.fandom.com/wiki/Quaternions. */

			const auto t = ComponentType( 2 ) * Math::Cross( xyz, vector_to_rotate );
			return vector_to_rotate + w * t - Math::Cross( t, xyz );
		}

		constexpr Quaternion Normalized() const
		{
			return *this / Magnitude();
		}

		constexpr Quaternion& Normalize()
		{
			return *this /= Magnitude();
		}

		/* With self. */
		constexpr ComponentType Dot() const
		{
			ComponentType result( 0 );

			return w * w + xyz.Dot();
		}

		constexpr Quaternion Conjugate() const
		{
			return Quaternion( -xyz, w );
		}

		/* Sets this quaternion equal to its conjugate (i.e, [-x -y -z w]). */
		constexpr Quaternion& SetToConjugate()
		{
			xyz = -xyz;

			return *this;
		}

		constexpr Quaternion Inverse() const
		{
			return Conjugate() / Dot();
		}

		/* Assumes *this is a unit quaternion. Faster than Inverse().
		 * Simply returns the conjugate since inverse = conjugate / magnitude, where magnitude = 1 for a unit quaternion. */
		constexpr Quaternion Inverse_Normalized() const
		{
			ASSERT_DEBUG_ONLY( IsNormalized() && R"(Quaternion::Inverse_Normalized() : The quaternion "*this" is not normalized!)" );

			return Conjugate();
		}

		constexpr Quaternion& Invert()
		{
			return *this = Inverse();
		}

		/* Sets this quaternion equal to its inverse, which is its conjugate since a unit quaternion is assumed. (i.e, [w -x -y -z]). */
		constexpr Quaternion& Invert_Normalized()
		{
			return SetToConjugate();
		}

		/* Assumes unit quaternions.
		 * Returns a quaternion d such that d*a=b (where a = *this Quaternion).
		 * Here, d is the angular displacement between *this and the other. */
		constexpr Quaternion DifferenceBetween( const Quaternion& b ) const
		{
			return b * Inverse();
		}

		/* Taken from https://gamemath.com/book/orient.html#quaternions. */
		constexpr Quaternion Exp( const ComponentType exponent ) const
		{
			ASSERT_DEBUG_ONLY( IsNormalized() && R"(Quaternion::Exp() : The quaternion "*this" is not normalized!)" );
			
			Quaternion result( *this );

			// Check for the case of an identity quaternion. This will protect against divide by zero.
			if( Abs( result.w ) < ComponentType{ 0.9999 } )
			{
				// Extract the half angle alpha (alpha = theta/2).
				const RadiansType alpha = Acos( result.w );

				// Compute new alpha, w & xyz values.
				const RadiansType new_alpha = alpha * exponent;

				result.w = Cos( new_alpha );
				result.xyz *= SinFromCos( result.w ) / SinFromCos( w ); // Optimize: Use SinFromCos( w ) instead of Sin( Alpha ).
			}

			return result;
		}

		template< std::floating_point ComponentType >
		static constexpr Quaternion< ComponentType > LookRotation_Naive( const Vector< ComponentType, 3 >& to_target_normalized, const Vector< ComponentType, 3 >& world_up_normalized = Vector< ComponentType, 3 >::Up() )
		{
			ASSERT_DEBUG_ONLY( to_target_normalized.IsNormalized() && R"(Math::LookRotation(): "to_target_normalized" is not normalized!)" );
			ASSERT_DEBUG_ONLY(  world_up_normalized.IsNormalized() && R"(Math::LookRotation():  "world_up_normalized" is not normalized!)" );

			const auto to_right_normalized = Math::Cross( world_up_normalized, to_target_normalized ).Normalized();
			const auto to_up_normalized    = Math::Cross( to_target_normalized, to_right_normalized ).Normalized();

			return Quaternion< ComponentType >
			{
				Math::Angle( to_target_normalized, Vector< ComponentType, 3 >::Forward() ),				// 1 Dot() + 1 Acos().
				Math::Cross( to_target_normalized, Vector< ComponentType, 3 >::Forward() ).Normalized() // 1 Cross() + 1 Vector3::Normalized().
			};
		}

		template< std::floating_point ComponentType >
		static constexpr Quaternion< ComponentType > LookRotation( const Vector< ComponentType, 3 >& to_target_normalized, const Vector< ComponentType, 3 >& world_up_normalized = Vector< ComponentType, 3 >::Up() )
		{
			ASSERT_DEBUG_ONLY( to_target_normalized.IsNormalized() && R"(Math::LookRotation(): "to_target_normalized" is not normalized!)" );
			ASSERT_DEBUG_ONLY(  world_up_normalized.IsNormalized() && R"(Math::LookRotation():  "world_up_normalized" is not normalized!)" );

			const auto to_right_normalized = Math::Cross( world_up_normalized, to_target_normalized ).Normalized();
			const auto to_up_normalized    = Math::Cross( to_target_normalized, to_right_normalized ).Normalized();

			return MatrixToQuaternion( Matrix< ComponentType, 3, 3 >( to_right_normalized, to_up_normalized, to_target_normalized ) ).Normalized();
		}

		union
		{
			struct { VectorType xyz; };
			struct { ComponentType x, y, z; };
		};
		ComponentType w;
	};

	/* Arithmetic Operations: Binary operators (with a scalar), of the the form scalar-operator-quaternion. */
	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > operator* ( const ComponentType scalar, Quaternion< ComponentType > quaternion )
	{
		return quaternion * scalar;
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > operator/ ( const ComponentType scalar, Quaternion< ComponentType > quaternion )
	{
		return quaternion / scalar;
	}

	template< std::floating_point ComponentType >
	constexpr ComponentType Dot( const Quaternion< ComponentType >& q1, const Quaternion< ComponentType >& q2 )
	{
		return q1.w * q2.w + Dot( q1.xyz, q2.xyz );
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > Lerp( const Quaternion< ComponentType > q1, const Quaternion< ComponentType >& q2, const ComponentType t )
	{
		return ( ComponentType{ 1 } - t ) * q1 + t * q2;
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > Nlerp( const Quaternion< ComponentType > q1, const Quaternion< ComponentType >& q2, const ComponentType t )
	{
		ASSERT_DEBUG_ONLY( q1.IsNormalized() && R"(Quaternion::Nlerp() : The quaternion q1 is not normalized!)" );
		ASSERT_DEBUG_ONLY( q2.IsNormalized() && R"(Quaternion::Nlerp() : The quaternion q2 is not normalized!)" );

		return Lerp( q1, q2, t ).Normalized();
	}

	/* The algebraic derivation. Included for completion. Not to be used in production code, as it is quite inefficient. */
	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > Slerp_Naive( const Quaternion< ComponentType >& q1, const Quaternion< ComponentType >& q2, const ComponentType t )
	{
		ASSERT_DEBUG_ONLY( q1.IsNormalized() && R"(Quaternion::Slerp_Naive() : The quaternion q1 is not normalized!)" );
		ASSERT_DEBUG_ONLY( q2.IsNormalized() && R"(Quaternion::Slerp_Naive() : The quaternion q2 is not normalized!)" );

		return q1.DifferenceBetween( q2 ).Exp( t ) * q1;
	}

	/* Geometric derivation. Computationally more efficient than the naive (algebraic) derivation. */
	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > Slerp( const Quaternion< ComponentType >& q1, Quaternion< ComponentType > q2, const ComponentType t )
	{
		ASSERT_DEBUG_ONLY( q1.IsNormalized() && R"(Quaternion::Slerp() : The quaternion q1 is not normalized!)" );
		ASSERT_DEBUG_ONLY( q2.IsNormalized() && R"(Quaternion::Slerp() : The quaternion q2 is not normalized!)" );

		using RadiansType = Radians< ComponentType >;

		const auto dot = Math::Dot( q1, q2 );

		if( dot < 0 )
			/* Negate one of the input quaternions, to take the shorter 4D "arc". */
			q2 = -q2;

		const auto cos_theta = dot < 0 ? -dot : +dot;

		if( cos_theta > Engine::TypeTraits< ComponentType >::OneMinusEpsilon() )
			/* Quaternions are too close; Revert back to a simple Nlerp(). */
			return Nlerp( q1, q2, t );

		const auto sin_theta = SinFromCos( cos_theta ); // sqrt() is faster than sin().

		/* Let's leverage the fact that now we have sin_theta & cos_theta in order to more efficiently find theta, compared to using acos/asin( theta ). */
		const RadiansType theta( Atan2( sin_theta, cos_theta ) );

		/* Compute 1 / sin(theta) once here, to not have to incur division of the 4 components by sin(theta) later on. */
		const auto one_over_sin_theta = ComponentType{ 1 } / Sin( theta );

		return ( q1 * Sin( ( ComponentType{ 1 } - t ) * theta ) + q2 * Sin( t * theta ) )
			* one_over_sin_theta;
	}

	template< std::floating_point ComponentType >
	constexpr Matrix< ComponentType, 4, 4 > QuaternionToMatrix( const Quaternion< ComponentType >& quaternion )
	{
		ASSERT_DEBUG_ONLY( quaternion.IsNormalized() && R"(Math::QuaternionToMatrix(): The quaternion is not normalized!)" );

		const auto two_x2  = ComponentType( 2 ) * quaternion.x * quaternion.x;
		const auto two_y2  = ComponentType( 2 ) * quaternion.y * quaternion.y;
		const auto two_z2  = ComponentType( 2 ) * quaternion.z * quaternion.z;
		const auto two_x_y = ComponentType( 2 ) * quaternion.x * quaternion.y;
		const auto two_x_z = ComponentType( 2 ) * quaternion.x * quaternion.z;
		const auto two_y_z = ComponentType( 2 ) * quaternion.y * quaternion.z;
		const auto two_w_x = ComponentType( 2 ) * quaternion.w * quaternion.x;
		const auto two_w_y = ComponentType( 2 ) * quaternion.w * quaternion.y;
		const auto two_w_z = ComponentType( 2 ) * quaternion.w * quaternion.z;

		return Matrix< ComponentType, 4, 4 >
		(
			{
				ComponentType( 1 ) - two_y2 - two_z2,	two_x_y + two_w_z,						two_x_z - two_w_y,						ComponentType( 0 ),
				two_x_y - two_w_z,						ComponentType( 1 ) - two_x2 - two_z2,	two_y_z + two_w_x,						ComponentType( 0 ),
				two_x_z + two_w_y,						two_y_z - two_w_x,						ComponentType( 1 ) - two_x2 - two_y2,	ComponentType( 0 ),
				ComponentType( 0 ),						ComponentType( 0 ),						ComponentType( 0 ),						ComponentType( 1 )
			}
		);
	}

	template< std::floating_point ComponentType >
	constexpr Matrix< ComponentType, 3, 3 > QuaternionToMatrix3x3( const Quaternion< ComponentType >& quaternion )
	{
		ASSERT_DEBUG_ONLY( quaternion.IsNormalized() && R"(Math::QuaternionToMatrix(): The quaternion is not normalized!)" );

		const auto two_x2  = ComponentType( 2 ) * quaternion.x * quaternion.x;
		const auto two_y2  = ComponentType( 2 ) * quaternion.y * quaternion.y;
		const auto two_z2  = ComponentType( 2 ) * quaternion.z * quaternion.z;
		const auto two_x_y = ComponentType( 2 ) * quaternion.x * quaternion.y;
		const auto two_x_z = ComponentType( 2 ) * quaternion.x * quaternion.z;
		const auto two_y_z = ComponentType( 2 ) * quaternion.y * quaternion.z;
		const auto two_w_x = ComponentType( 2 ) * quaternion.w * quaternion.x;
		const auto two_w_y = ComponentType( 2 ) * quaternion.w * quaternion.y;
		const auto two_w_z = ComponentType( 2 ) * quaternion.w * quaternion.z;

		return Matrix< ComponentType, 3, 3 >
		(
			{
				ComponentType( 1 ) - two_y2 - two_z2,	two_x_y + two_w_z,						two_x_z - two_w_y,					
				two_x_y - two_w_z,						ComponentType( 1 ) - two_x2 - two_z2,	two_y_z + two_w_x,					
				two_x_z + two_w_y,						two_y_z - two_w_x,						ComponentType( 1 ) - two_x2 - two_y2
			}
		);
	}

	/* Source: https://gamemath.com/book/orient.html#euler_to_matrix. */
	template< std::floating_point ComponentType, std::size_t MatrixSize > requires( MatrixSize >= 3 )
	constexpr Quaternion< ComponentType > MatrixToQuaternion( const Matrix< ComponentType, MatrixSize, MatrixSize >& matrix )
	{
		ComponentType w, x, y, z;

		// Determine which of w, x, y, or z has the largest absolute value.
		ComponentType four_w_squared_minus_1 = matrix[ 0 ][ 0 ] + matrix[ 1 ][ 1 ] + matrix[ 2 ][ 2 ];
		ComponentType four_x_squared_minus_1 = matrix[ 0 ][ 0 ] - matrix[ 1 ][ 1 ] - matrix[ 2 ][ 2 ];
		ComponentType four_y_squared_minus_1 = matrix[ 1 ][ 1 ] - matrix[ 0 ][ 0 ] - matrix[ 2 ][ 2 ];
		ComponentType four_z_squared_minus_1 = matrix[ 2 ][ 2 ] - matrix[ 0 ][ 0 ] - matrix[ 1 ][ 1 ];

		std::size_t biggest_index = 0;
		ComponentType four_biggest_squared_minus1 = four_w_squared_minus_1;

		if( four_x_squared_minus_1 > four_biggest_squared_minus1 )
		{
			four_biggest_squared_minus1 = four_x_squared_minus_1;
			biggest_index = 1;
		}
		if( four_y_squared_minus_1 > four_biggest_squared_minus1 )
		{
			four_biggest_squared_minus1 = four_y_squared_minus_1;
			biggest_index = 2;
		}
		if( four_z_squared_minus_1 > four_biggest_squared_minus1 )
		{
			four_biggest_squared_minus1 = four_z_squared_minus_1;
			biggest_index = 3;
		}

		// Perform square root and division.
		ComponentType biggestVal = Sqrt( four_biggest_squared_minus1 + ComponentType( 1 ) ) * ComponentType( 0.5 );
		ComponentType mult = ComponentType( 0.25 ) / biggestVal;

		// Apply table to compute quaternion values
		switch( biggest_index )
		{
			case 0:
				w = biggestVal;
				x = ( matrix[ 1 ][ 2 ] - matrix[ 2 ][ 1 ] ) * mult;
				y = ( matrix[ 2 ][ 0 ] - matrix[ 0 ][ 2 ] ) * mult;
				z = ( matrix[ 0 ][ 1 ] - matrix[ 1 ][ 0 ] ) * mult;
				break;

			case 1:
				x = biggestVal;
				w = ( matrix[ 1 ][ 2 ] - matrix[ 2 ][ 1 ] ) * mult;
				y = ( matrix[ 0 ][ 1 ] + matrix[ 1 ][ 0 ] ) * mult;
				z = ( matrix[ 2 ][ 0 ] + matrix[ 0 ][ 2 ] ) * mult;
				break;

			case 2:
				y = biggestVal;
				w = ( matrix[ 2 ][ 0 ] - matrix[ 0 ][ 2 ] ) * mult;
				x = ( matrix[ 0 ][ 1 ] + matrix[ 1 ][ 0 ] ) * mult;
				z = ( matrix[ 1 ][ 2 ] + matrix[ 2 ][ 1 ] ) * mult;
				break;

			case 3:
				z = biggestVal;
				w = ( matrix[ 0 ][ 1 ] - matrix[ 1 ][ 0 ] ) * mult;
				x = ( matrix[ 2 ][ 0 ] + matrix[ 0 ][ 2 ] ) * mult;
				y = ( matrix[ 1 ][ 2 ] + matrix[ 2 ][ 1 ] ) * mult;
				break;
		}

		return { x, y, z, w };
	}

	/* Source: https://gamemath.com/book/orient.html#quaternion_to_euler_angles. */
	template< std::floating_point ComponentType >
	constexpr void QuaternionToEuler( const Quaternion< ComponentType >& quaternion, Radians< ComponentType >& heading_around_y, Radians< ComponentType >& pitch_around_x, Radians< ComponentType >& bank_around_z )
	{
		/* Method is to convert a matrix to euler angles, where the matrix is the angle-axis rotation matrix re-arranged to use x, y, z, w of a Quaternion.
		 * So basically, it is kind it is QuaternionToMatrix() & then MatrixToEuler(). */

		ASSERT_DEBUG_ONLY( quaternion.IsNormalized() && R"(Math::QuaternionToEuler(): The quaternion is not normalized!)" );

		const auto x = quaternion.x;
		const auto y = quaternion.y;
		const auto z = quaternion.z;
		const auto w = quaternion.w;

		ComponentType sin_pitch = ComponentType ( -2 ) * ( y * z - w * x );

		// Check for gimbal lock, giving slight tolerance for numerical imprecision.
		if( Abs( sin_pitch ) > TypeTraits< ComponentType >::OneMinusEpsilon() )
		{
			pitch_around_x = Radians( Constants< ComponentType >::Pi_Over_Two() * sin_pitch );

			// Compute heading, slam bank to zero.
			heading_around_y = Atan2( -x * z + w * y, ComponentType( 0.5 ) - y * y - z * z );
			bank_around_z    = Radians( ComponentType( 0 ) );

		}
		else // Compute angles.
		{
			pitch_around_x   = Asin( sin_pitch );
			heading_around_y = Atan2( x * z + w * y, ComponentType( 0.5 ) - x * x - y * y );
			bank_around_z    = Atan2( x * y + w * z, ComponentType( 0.5 ) - x * x - z * z );
		}
	}

	template< std::floating_point ComponentType >
	constexpr void QuaternionToEuler( const Quaternion< ComponentType >& quaternion, Vector< Radians< ComponentType >, 3 >& euler_angles )
	{
		// Must provide parameters in the correct order heading, pitch, yaw, i.e., y, x, z.
		QuaternionToEuler( quaternion, euler_angles[ 1 ], euler_angles[ 0 ], euler_angles[ 2 ] );
	}

	template< std::floating_point ComponentType >
	constexpr void QuaternionToEuler( const Quaternion< ComponentType >& quaternion, Degrees< ComponentType >& heading_around_y, Degrees< ComponentType >& pitch_around_x, Degrees< ComponentType >& bank_around_z )
	{
		Radians< ComponentType > heading, pitch, bank;
		QuaternionToEuler( quaternion, heading, pitch, bank );
		heading_around_y = Degrees( heading );
		pitch_around_x   = Degrees( pitch );
		bank_around_z    = Degrees( bank );
	}

	template< std::floating_point ComponentType >
	constexpr void QuaternionToEuler( const Quaternion< ComponentType >& quaternion, Vector< Degrees< ComponentType >, 3 >& euler_angles )
	{
		// Must provide parameters in the correct order heading, pitch, yaw, i.e., y, x, z.
		QuaternionToEuler( quaternion, euler_angles[ 1 ], euler_angles[ 0 ], euler_angles[ 2 ] );
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > EulerToQuaternion( const Radians< ComponentType > heading_around_y, const Radians< ComponentType > pitch_around_x, const Radians< ComponentType > bank_around_z )
	{
		const auto half_heading( heading_around_y / ComponentType( 2 ) );
		const auto half_pitch( pitch_around_x / ComponentType( 2 ) );
		const auto half_bank( bank_around_z / ComponentType( 2 ) );

		const auto cos_half_heading = Cos( half_heading );
		const auto cos_half_pitch   = Cos( half_pitch	);
		const auto cos_half_bank    = Cos( half_bank	);

		const auto sin_half_heading = Sin( half_heading );
		const auto sin_half_pitch   = Sin( half_pitch	);
		const auto sin_half_bank    = Sin( half_bank	);

		const auto sin_half_heading_sin_half_pitch = sin_half_heading * sin_half_pitch;
		const auto cos_half_pitch_sin_half_bank    = cos_half_pitch   * sin_half_bank;
		const auto cos_half_heading_cos_half_bank  = cos_half_heading * cos_half_bank;
		const auto cos_half_heading_sin_half_bank  = cos_half_heading * sin_half_bank;

		return
		{
			/* x = */  cos_half_heading_cos_half_bank  * sin_half_pitch + sin_half_heading * cos_half_pitch_sin_half_bank,
			/* y = */ -cos_half_heading_sin_half_bank  * sin_half_pitch + sin_half_heading * cos_half_pitch * cos_half_bank,
			/* z = */ -sin_half_heading_sin_half_pitch * cos_half_bank  + cos_half_heading_sin_half_bank * cos_half_pitch,

			/* w = */  cos_half_heading_cos_half_bank  * cos_half_pitch + sin_half_heading_sin_half_pitch * sin_half_bank
		};
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > EulerToQuaternion( const Vector< Radians< ComponentType >, 3 > euler_angles )
	{
		// Must provide parameters in the correct order heading, pitch, yaw, i.e., y, x, z.
		return EulerToQuaternion< ComponentType >( euler_angles[ 1 ], euler_angles[ 0 ], euler_angles[ 2 ] );
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > EulerToQuaternion( const Degrees< ComponentType > heading_around_y, const Degrees< ComponentType > pitch_around_x, const Degrees< ComponentType > bank_around_z )
	{
		return EulerToQuaternion< ComponentType >( Radians( heading_around_y ), Radians( pitch_around_x ), Radians( bank_around_z ) );
	}

	template< std::floating_point ComponentType >
	constexpr Quaternion< ComponentType > EulerToQuaternion( const Vector< Degrees< ComponentType >, 3 > euler_angles )
	{
		// Must provide parameters in the correct order heading, pitch, yaw, i.e., y, x, z.
		return EulerToQuaternion< ComponentType >( euler_angles[ 1 ], euler_angles[ 0 ], euler_angles[ 2 ] );
	}
}

namespace Engine
{
	using Quaternion  = Math::Quaternion< float  >;
	using QuaternionD = Math::Quaternion< double >;
}
