#pragma once

// Project Includes.
#include "Math/Polar.h"
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::Indexed::SphereTemplate
{
	/* Ordering:
	 * 1) North pole.
	 * 3) Rings (in descending order, from north to south). Inside a ring: Going clockwise, to adhere to left-handed system.
	 * 3) South pole. */
	template< std::uint8_t LongitudeCount = 20, float Diameter = 1.0f > requires( LongitudeCount >= 3 )
	auto Positions()
	{
		constexpr std::uint8_t latitude_count             = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count        = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t non_pole_ring_vertex_count = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = non_pole_ring_count * non_pole_ring_vertex_count + 2; // +2 for the poles. u coordinate of the poles do not matter as it is a singularity.
		std::array< Vector3, vertex_count > positions;

		constexpr float radius        = Diameter / 2.0f;
		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / LongitudeCount;

		std::uint8_t index = 0;

		constexpr Vector3 north_pole = Vector3( 0.0f, +radius, 0.0f );
		constexpr Vector3 south_pole = Vector3( 0.0f, -radius, 0.0f );

		/* North pole: */
		positions[ index++ ] = north_pole;

		using namespace Math::Literals;

		Math::Polar3_Spherical_Game spherical_coordinates( radius, 0.0_rad, -Constants< Radians >::Pi_Over_Two() );

		for( std::uint8_t ring_index = 0; ring_index < non_pole_ring_count; ring_index++ )
		{
			spherical_coordinates.Pitch() += delta_angle;

			for( std::uint8_t longitude_index = 0; longitude_index < LongitudeCount; longitude_index++ )
			{
				spherical_coordinates.Heading() = delta_angle * longitude_index;
				positions[ index++ ] = Math::ToVector3( spherical_coordinates );
			}

			/* Duplicate the starting vertex (u=0) to allow u=1. */
			spherical_coordinates.Heading() = 0.0_rad; // Reset.
			positions[ index++ ] = Math::ToVector3( spherical_coordinates );
		}

		/* South pole: */
		positions[ index++ ] = south_pole;

		return positions;
	};
}
