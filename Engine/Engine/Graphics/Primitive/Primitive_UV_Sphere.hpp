#pragma once

// Project Includes.
#include "Math/Polar.h"
#include "Math/Vector.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::Indexed::UVSphereTemplate
{
	/* Ordering:
	 * 1) North pole.
	 * 3) Rings (in descending order, from north to south). Inside a ring: Going clockwise, to adhere to left-handed system.
	 * 3) South pole. */
	template< std::uint8_t LongitudeCount = 20, float Diameter = 1.0f > requires( LongitudeCount >= 3 )
	auto Positions()
	{
		constexpr std::uint8_t latitude_count      = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count   = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.
		std::array< Vector3, vertex_count > positions;

		constexpr float radius          = Diameter / 2.0f;
		constexpr Radians delta_heading = Constants< Radians >::Two_Pi() / LongitudeCount;
		constexpr Radians delta_pitch   = Constants< Radians >::Pi()     / ( LongitudeCount - 1 );

		std::uint16_t index = 0;

		constexpr Vector3 north_pole = Vector3( 0.0f, +radius, 0.0f );
		constexpr Vector3 south_pole = Vector3( 0.0f, -radius, 0.0f );

		/* North pole: */
		for( std::uint8_t pole_vertices_index = 0; pole_vertices_index < ring_vertex_count; pole_vertices_index++ )
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
		for( std::uint8_t pole_vertices_index = 0; pole_vertices_index < ring_vertex_count; pole_vertices_index++ )
			positions[ index++ ] = south_pole;

		return positions;
	};

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto Indices()
	{
		using IndexType = std::uint16_t;

		constexpr std::uint8_t latitude_count      = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count   = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.

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
			indices[ array_index++ ] = i;
			indices[ array_index++ ] = stride + i;
			indices[ array_index++ ] = stride + i + 1;
		}

		/* Side vertices: */

		for( IndexType side_band_index = 0; side_band_index < side_band_count; side_band_index++ )
		{
			const unsigned int first_vertex = stride + ( unsigned int )side_band_index * stride;
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
			constexpr unsigned int first_south_pole_vertex = vertex_count - stride;
			constexpr unsigned int first_vertex_of_last_band = first_south_pole_vertex - stride;
			for( auto i = 0; i < LongitudeCount; i++ )
			{
				indices[ array_index++ ] = first_vertex_of_last_band + i;
				indices[ array_index++ ] = first_south_pole_vertex + i;
				indices[ array_index++ ] = first_vertex_of_last_band + i + 1;
			}
		}

		return indices;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	constexpr auto UVs()
	{
		constexpr std::uint8_t latitude_count      = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count   = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.
		std::array< Vector2, vertex_count > uvs;

		constexpr float delta_u = 1.0f / LongitudeCount;
		constexpr float delta_v = 1.0f / ( LongitudeCount - 1 );

		std::uint16_t index = 0;

		/* North pole: */
		for( std::uint8_t pole_vertices_index = 0; pole_vertices_index < ring_vertex_count; pole_vertices_index++ )
			uvs[ index++ ] = Vector2( delta_u * ( 0.5f + ( float )pole_vertices_index ), 1.0f );

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
		for( std::uint8_t pole_vertices_index = 0; pole_vertices_index < ring_vertex_count; pole_vertices_index++ )
			uvs[ index++ ] = Vector2( delta_u * ( 0.5f + ( float )pole_vertices_index ), 0.0f );

		return uvs;
	}

	/* Check Positions() for vertex ordering.
	 * Normals are smoothed out over the sphere surface. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto Normals()
	{
		constexpr std::uint8_t latitude_count      = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count   = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.
		std::array< Vector3, vertex_count > normals;

		const auto positions( Positions< LongitudeCount >() );

		for( std::uint16_t i = 0; i < vertex_count; i++ )
			normals[ i ] = positions[ i ].Normalized();

		return normals;
	}

	/* Check Positions() for vertex ordering.
	 * Tangents are smoothed out over the sphere surface (because normals are smoothed out over the sphere surface). */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
		auto Tangents()
	{
		constexpr std::uint8_t latitude_count = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.
		std::array< Vector3, vertex_count > tangents;

		const auto normals( Normals< LongitudeCount >() );

		/* Think of a meridian/longitude as forming a plane.
		 * That plane's normal is the tangent.
		 * Normal of the plane/meridian/longitude is defined by the left-hand rule, with the hand curving from the south pole to the north pole.
		 *
		 * An easy way to determine the tangent is to first project the normal to the XZ plane and then cross it with world up vector.
		 *
		 * Also, calculating the tangents for a ring (say, the north pole ring, as it comes first) will suffice,
		 * as each vertex corresponding to the same meridian/longitude has the same tangent.
		 */

		std::uint16_t index = 0;

		/* Actually calculate the tangents once for the north pole ring: */
		for( ; index < ring_vertex_count; index++ )
		{
			const auto index_of_vertex_in_first_non_pole_ring( ring_vertex_count + index );

			const auto& normal( normals[ index_of_vertex_in_first_non_pole_ring ] );
			const auto normal_projected_to_xz( Vector3( normal ).SetY( 0.0f ).Normalized() );
			tangents[ index ] = Math::Cross( normal, normal_projected_to_xz ).Normalized();
		}

		/* Now copy the tangents to other rings: */
		for( ; index < vertex_count; index++ )
			tangents[ index ] = tangents[ index % ring_vertex_count ];

		return tangents;
	}

	/* Check Positions() for vertex ordering.
	 * Bitangents are smoothed out over the sphere surface (because normals & tangents are smoothed out over the sphere surface). */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto Bitangents()
	{
		constexpr std::uint8_t latitude_count      = LongitudeCount;
		constexpr std::uint8_t non_pole_ring_count = LongitudeCount - 2; // -2 to exclude the poles, which count as latitudes in a uv-sphere.
		constexpr std::uint8_t ring_vertex_count   = LongitudeCount + 1; // +1 to include the u = 1 vertex, which shares the same position as the u = 0 vertex.

		constexpr std::uint16_t vertex_count = LongitudeCount * ring_vertex_count; // Poles also need as many as ring_vertex_count vertices for uvs & other attribs.
		std::array< Vector3, vertex_count > bitangents;

		const auto normals( Normals< LongitudeCount >() );
		const auto tangents( Tangents< LongitudeCount >() );

		/* Calculating the bitangents for a meridian/longitude (say, the 1st one) will suffice, 
		 * as each vertex corresponding to the same ring/latitude has the same bitangent.
		 */

		std::uint16_t index = 0;

		/* Actually calculate the bitangents once for the north pole ring: */
		for( ; index < ring_vertex_count; index++ )
		{
			const auto index_of_vertex_in_first_non_pole_ring( ring_vertex_count + index );

			const auto& normal( normals[ index_of_vertex_in_first_non_pole_ring ] );
			bitangents[ index ] = Math::Cross( tangents[ index_of_vertex_in_first_non_pole_ring ], normals[ index_of_vertex_in_first_non_pole_ring ] ).Normalized();
		}

		/* Now copy the bitangents to other rings: */
		for( ; index < vertex_count; index++ )
			bitangents[ index ] = bitangents[ index % ring_vertex_count ];

		return bitangents;
	}
}
