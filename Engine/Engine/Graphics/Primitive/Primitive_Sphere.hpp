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

		constexpr float radius          = Diameter / 2.0f;
		constexpr Radians delta_heading = Constants< Radians >::Two_Pi() / LongitudeCount;
		constexpr Radians delta_pitch   = Constants< Radians >::Pi()     / LongitudeCount;

		std::uint16_t index = 0;

		constexpr Vector3 north_pole = Vector3( 0.0f, +radius, 0.0f );
		constexpr Vector3 south_pole = Vector3( 0.0f, -radius, 0.0f );

		/* North pole: */
		positions[ index++ ] = north_pole;

		using namespace Math::Literals;

		Math::Polar3_Spherical_Game spherical_coordinates( radius, 0.0_rad, -Constants< Radians >::Pi_Over_Two() );

		/* NOTE: Instead of using a Polar3_Spherical_Game here, the individual heading/pitch and their corresponding sin/cos values can be tracked & modified manually. 
		 * While this would potentially be faster due to caching the sin/cos of the pitch & the xz_projection_of_r values [per ring],
		 * the performance gains would be minimal for the default longitude/latitude of 20 (nano-micro seconds probably).
		 * Also, as the returned positions & other attributes are cached as a static variable for the default sphere, this optimization is not necessary,
		 * and the explicit and longer LOC nature of the code is a negative here. */

		/* Non-pole rings: */
		for( std::uint8_t ring_index = 0; ring_index < non_pole_ring_count; ring_index++ )
		{
			spherical_coordinates.Pitch() += delta_pitch;

			for( std::uint8_t longitude_index = 0; longitude_index < LongitudeCount; longitude_index++ )
			{
				spherical_coordinates.Heading() = delta_heading * longitude_index;
				positions[ index++ ] = Math::ToVector3( spherical_coordinates );
			}

			/* Duplicate the starting vertex (u=0) of the ring to allow u=1. */
			spherical_coordinates.Heading() = 0.0_rad; // Reset.
			positions[ index++ ] = Math::ToVector3( spherical_coordinates );
		}

		/* South pole: */
		positions[ index++ ] = south_pole;

		return positions;
	};

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto Indices()
	{
		using IndexType = std::uint16_t;

		constexpr std::uint8_t latitude_count             = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count        = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t non_pole_ring_vertex_count = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = non_pole_ring_count * non_pole_ring_vertex_count + 2; // +2 for the poles. u coordinate of the poles do not matter as it is a singularity.

		/* There are (N-1) bands for N longitudes (including the poles as longitudes).
		 * The top and bottom bands are triangular and counted separately.
		 * Remaining (N-3) bands have quads. We call those the "side" vertices/indices. */
		constexpr IndexType side_band_triangle_count = 2 * LongitudeCount;
		constexpr IndexType side_band_index_count    = 3 * side_band_triangle_count;
		constexpr IndexType side_band_count          = LongitudeCount - 3;
		constexpr IndexType cap_index_count          = 3 * LongitudeCount;
		constexpr IndexType total_index_count        = 2 * cap_index_count + side_band_count * side_band_index_count;

		std::array< unsigned int, total_index_count > indices;

		constexpr unsigned int stride = LongitudeCount + 1;

		IndexType array_index = 0;

		/* Top cap: */
		for( auto i = 0; i < LongitudeCount; i++ )
		{
			indices[ array_index++ ] = 0;
			indices[ array_index++ ] = i + 1;
			indices[ array_index++ ] = i + 2;
		}

		/* Side vertices: */

		for( IndexType side_band_index = 0; side_band_index < side_band_count; side_band_index++ )
		{
			const unsigned int first_vertex = 1 + ( unsigned int )side_band_index * stride;
			for( std::uint8_t i = 0; i < LongitudeCount; i++ )
			{
				indices[ array_index++ ] = first_vertex + i;
				indices[ array_index++ ] = first_vertex + i + stride;
				indices[ array_index++ ] = first_vertex + i + 1;
				indices[ array_index++ ] = first_vertex + i + 1;
				indices[ array_index++ ] = first_vertex + i + stride;
				indices[ array_index++ ] = first_vertex + i + 1 + stride;
			}
		}

		/* Bottom cap: */
		{
			constexpr unsigned int last_vertex = vertex_count - 1;
			constexpr unsigned int first_vertex_of_last_band = last_vertex - stride;
			for( auto i = 0; i < LongitudeCount; i++ )
			{
				indices[ array_index++ ] = first_vertex_of_last_band + i;
				indices[ array_index++ ] = last_vertex;
				indices[ array_index++ ] = first_vertex_of_last_band + i + 1;
			}
		}

		return indices;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	constexpr auto UVs()
	{
		constexpr std::uint8_t latitude_count             = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count        = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t non_pole_ring_vertex_count = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = non_pole_ring_count * non_pole_ring_vertex_count + 2; // +2 for the poles. u coordinate of the poles do not matter as it is a singularity.
		std::array< Vector2, vertex_count > uvs;

		constexpr float delta_u = 1.0f / LongitudeCount;
		constexpr float delta_v = 1.0f / ( LongitudeCount - 1 );

		std::uint16_t index = 0;

		constexpr Vector2 north_pole = Vector2( 0.0f, 1.0f );
		constexpr Vector2 south_pole = Vector2( 0.0f, 0.0f );

		/* North pole: */
		uvs[ index++ ] = north_pole;

		/* NOTE: See the note in Positions(). */


		/* Non-pole rings: */
		for( std::uint8_t ring_index = 0; ring_index < non_pole_ring_count; ring_index++ )
		{
			const float v = 1.0f - ( ( 1 + ring_index ) * delta_v );

			for( std::uint8_t longitude_index = 0; longitude_index < LongitudeCount; longitude_index++ )
				uvs[ index++ ] = Vector2( longitude_index * delta_u, v );

			/* Duplicate the starting vertex (u=0) of the ring to allow u=1. */
			uvs[ index++ ] = Vector2( 1.0f, v );
		}

		/* South pole: */
		uvs[ index++ ] = south_pole;

		return uvs;
	}
}
