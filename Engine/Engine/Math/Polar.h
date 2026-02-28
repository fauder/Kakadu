#pragma once

// Engine Includes.
#include "Core/Initialization.h"
#include "Math/Angle.hpp"

namespace Kakadu::Math
{
	/* In classical mathematical notation (r, theta). X is right & Y is forward & Z is up (right-handed space.) */
	class Polar2
	{
	public:
	/* Constructors. */
		Polar2( Initialization::ZeroInitialization )
			:
			radius(),
			theta()
		{}

		Polar2( const float radius, Kakadu::Radians theta )
			:
			radius( radius ),
			theta( theta )
		{}

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Polar2 );

	/* Getters & Setters. */
		constexpr float Radius() const { return radius; }
		float& Radius() { return radius; }
		constexpr Kakadu::Radians Theta() const { return theta; }
		Kakadu::Radians& Theta() { return theta; }

	private:
		float radius;
		Kakadu::Radians theta;
	};

	/* Deviates from classical mathematical notation (r, theta, phi), more in-line with video game conventions/needs.
	 * In this convention, Y is up, X is right & Z is forward (<0,0,+1>) (left-handed).
	 * Uses (r, h, p), r = radius, h = heading, p = pitch.
	 * Heading = 0 -> facing forward (<0,0,+1>) (assuming Pitch is also zero). Its value increases -> clockwise rotation.
	 * Pitch   = 0 -> horizontal (on XZ plane). It's values increases -> clockwise rotation around X (i.e, facing "more downward", therefore also called the Angle of Declination).
	*/
	class Polar3_Spherical_Game
	{
	public:
	/* Constructors. */
		Polar3_Spherical_Game( Initialization::ZeroInitialization )
			:
			radius(),
			heading(),
			pitch()
		{}

		Polar3_Spherical_Game( const float radius, const Kakadu::Radians heading, const Kakadu::Radians pitch )
			:
			radius( radius ),
			heading( heading ),
			pitch( pitch )
		{}

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Polar3_Spherical_Game );

	/* Getters & Setters. */
		constexpr float Radius() const { return radius; }
		float& Radius() { return radius; }
		/* Heading = 0 means forward (<0,0,+1>) (assuming Pitch is also zero).
		 * Increasing Heading means clockwise rotation around the Y axis.
		 */
		constexpr Kakadu::Radians Heading() const { return heading; }
		/* Heading = 0 means forward (<0,0,+1>) (assuming Pitch is also zero).
		 * Increasing Heading means clockwise rotation around the Y axis.
		 */
		Kakadu::Radians& Heading() { return heading; }
		/* Pitch = 0 means horizontal.
		 * Also called the Angle of Declination, as Z decreases with increasing Pitch.
		 */
		constexpr Kakadu::Radians Pitch() const { return pitch; }
		/* Pitch = 0 means horizontal.
		 * Also called the Angle of Declination, as Z decreases with increasing Pitch.
		 */
		Kakadu::Radians& Pitch() { return pitch; }

	private:
		float radius;
		Kakadu::Radians heading;
		Kakadu::Radians pitch;
	};
}
