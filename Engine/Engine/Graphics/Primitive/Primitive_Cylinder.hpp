#pragma once

// Project Includes.
#include "Primitive_Circle.hpp"
#include "Math/Vector.hpp"
#include "Math/Quaternion.hpp"

// std Includes.
#include <array>

namespace Engine::Primitive::Indexed::CylinderTemplate
{
	/* Ordering:
	 * 1) Top cap vertices.
	 * 3) Side vertices.
	 * 3) Bottom cap vertices. */
	template< std::uint8_t LongitudeCount = 20, float Diameter = 1.0f, float Height = 2.0f > requires( LongitudeCount >= 3 )
	auto Positions()
	{
		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;
		std::array< Vector3, vertex_count > positions;

		constexpr float radius        = Diameter / 2.0f;
		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / LongitudeCount;
		constexpr float y_min         = -Height / 2.0f;
		constexpr float y_max         = +Height / 2.0f;

		const auto cap_positions( CircleTemplate::Positions< LongitudeCount, Diameter >() );

		/* Top cap: */
		std::transform( cap_positions.cbegin(), cap_positions.cend(),
						positions.begin(),
						[]( Vector3 position ){ return position.SetY( y_max ); } );

		/* Side vertices:
		 *
		 * Vertex ordering:
		 *  v1---v3
		 *   | \  |
		 *   |  \ |
		 *  v0---v2
		 *
		 * 1st triangle = v0-v1-v2
		 * 2nd triangle = v2-v1-v3
		 *
		 * NOTE: Last longitude will be duplicated to wrap UVs around the cylinder by having both u=0 & u=1 on same vertex positions (1 for the top & 1 for the bottom).
		 */

		std::uint8_t index = cap_vertex_count;

		{
			const std::uint8_t side_vertices_start_index = index;

			for( std::uint8_t i = 0; i < LongitudeCount; i++ )
			{
				const auto angle = delta_angle * i;
				/* v0 */ positions[ index++ ] = Vector3( radius * Math::Cos( angle ), y_min, radius * Math::Sin( angle ) );
				/* v1 */ positions[ index++ ] = Vector3( radius * Math::Cos( angle ), y_max, radius * Math::Sin( angle ) );
			}

			/* Duplicate the starting vertices (u=0) to allow u=1. */
			/* v0 */ positions[ index++ ] = positions[ side_vertices_start_index + 0 ];
			/* v1 */ positions[ index++ ] = positions[ side_vertices_start_index + 1 ];
		}

		/* Bottom cap: */

		std::transform( cap_positions.cbegin(), cap_positions.cend(),
						positions.begin() + index,
						[]( Vector3 position )
		{
			return Quaternion::RotateAroundZ_By_Pi().Transform( position.SetY( y_max ) );
		} );

		return positions;
	};

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto Indices()
	{
		using IndexType = std::uint16_t;

		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;

		/* N pairs of longitudes for N longitudes => 2 triangles per longitude-pair => 2N triangles for N longitues. */
		constexpr IndexType side_triangle_count = 2 * LongitudeCount;
		constexpr IndexType side_index_count    = 3 * side_triangle_count;
		constexpr IndexType cap_index_count     = 3 * ( LongitudeCount - 2 );
		constexpr IndexType total_index_count   = 2 * cap_index_count + side_index_count;

		std::array< unsigned int, total_index_count > indices;

		const auto cap_indices( CircleTemplate::Indices< LongitudeCount >() );

		/* Top cap: */
		std::copy_n( cap_indices.cbegin(), cap_index_count, indices.begin() );

		/* Side vertices: */

		std::uint8_t array_index = cap_index_count;

		for( std::uint8_t i = 0; i < LongitudeCount; i++ )
		{
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 0;
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 1;
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 2;
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 2;
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 1;
			indices[ array_index++ ] = cap_vertex_count + i * 2 + 3;
		}

		/* Bottom cap: */
		std::transform( cap_indices.cbegin(), cap_indices.cend(), 
						indices.begin() + array_index,
						[]( const unsigned int index ) { return cap_vertex_count + side_vertex_count + index; } );

		return indices;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	auto UVs()
	{
		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;

		std::array< Vector2, vertex_count > uvs;

		const auto cap_uvs( CircleTemplate::UVs< LongitudeCount >() );

		/* Top cap: */
		std::copy_n( cap_uvs.cbegin(), cap_vertex_count, uvs.begin() );

		/* Side vertices: */

		std::uint8_t index = cap_vertex_count;

		{
			constexpr float delta_u = 1.0f / LongitudeCount;

			for( std::uint8_t i = 0; i < LongitudeCount; i++ )
			{
				/* v0 */ uvs[ index++ ] = Vector2( i * delta_u, 0.0f );
				/* v1 */ uvs[ index++ ] = Vector2( i * delta_u, 1.0f );
			}

			/* Duplicate the starting vertices (u=0) to allow u=1. */
			/* v0 */ uvs[ index++ ] = Vector2( 1.0f, 0.0f );
			/* v1 */ uvs[ index++ ] = Vector2( 1.0f, 1.0f );
		}

		/* Bottom cap: */

		std::transform( cap_uvs.cbegin(), cap_uvs.cend(),
						uvs.begin() + cap_vertex_count + side_vertex_count,
						[]( const Vector2 uv )
		{
			return Quaternion::RotateAroundZ_By_Pi().Transform( Vector3( uv.X(), uv.Y(), 0.0f ) ).XY();
		} );

		return uvs;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	constexpr auto Normals()
	{
		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;

		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / LongitudeCount;

		std::array< Vector3, vertex_count > normals;

		/* Top cap: */
		std::fill_n( normals.begin(), cap_vertex_count, Vector3::Up() );

		/* Side vertices: */

		std::uint8_t index = cap_vertex_count;

		{
			const std::uint8_t side_vertices_start_index = index;

			for( std::uint8_t i = 0; i < LongitudeCount; i++ )
			{
				const auto angle = delta_angle * i;
				/* v0 */ normals[ index++ ] = Vector3( Math::Cos( angle ), 0.0f, Math::Sin( angle ) );
				/* v1 */ normals[ index++ ] = Vector3( Math::Cos( angle ), 0.0f, Math::Sin( angle ) );
			}

			/* Duplicate the starting vertices (u=0) to allow u=1. */
			/* v0 */ normals[ index++ ] = Vector3::Right();
			/* v1 */ normals[ index++ ] = Vector3::Right();
		}

		/* Bottom cap: */
		std::fill_n( normals.begin() + cap_vertex_count + side_vertex_count, cap_vertex_count, Vector3::Down() );

		return normals;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	constexpr auto Tangents()
	{
		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;

		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / LongitudeCount;

		std::array< Vector3, vertex_count > tangents;

		/* Top cap: */
		std::fill_n( tangents.begin(), cap_vertex_count, Vector3::Right() );

		/* Side vertices: */

		std::uint8_t index = cap_vertex_count;

		{
			const std::uint8_t side_vertices_start_index = index;

			for( std::uint8_t i = 0; i < LongitudeCount; i++ )
			{
				const auto angle = delta_angle * i;
				/* v0 */ tangents[ index++ ] = Vector3( -Math::Sin( angle ), 0.0f, Math::Cos( angle ) );
				/* v1 */ tangents[ index++ ] = Vector3( -Math::Sin( angle ), 0.0f, Math::Cos( angle ) );
			}

			/* Duplicate the starting vertices (u=0) to allow u=1. */
			/* v0 */ tangents[ index++ ] = Vector3::Right();
			/* v1 */ tangents[ index++ ] = Vector3::Right();
		}

		/* Bottom cap: */
		std::fill_n( tangents.begin() + cap_vertex_count + side_vertex_count, cap_vertex_count, Vector3::Left() );

		return tangents;
	}

	/* Check Positions() for vertex ordering. */
	template< std::uint8_t LongitudeCount = 20 > requires( LongitudeCount >= 3 )
	constexpr auto Bitangents()
	{
		constexpr std::uint8_t cap_vertex_count  = LongitudeCount;
		constexpr std::uint8_t side_vertex_count = 2 * ( LongitudeCount + 1 );
		constexpr std::uint8_t vertex_count      = cap_vertex_count * 2 + side_vertex_count;

		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / LongitudeCount;

		std::array< Vector3, vertex_count > bitangents;

		/* Top cap: */
		std::fill_n( bitangents.begin(), cap_vertex_count, Vector3::Forward() );

		/* Side vertices: */
		std::fill_n( bitangents.begin() + cap_vertex_count, side_vertex_count, Vector3::Up() );

		/* Bottom cap: */
		std::fill_n( bitangents.begin() + cap_vertex_count + side_vertex_count, cap_vertex_count, Vector3::Forward() );

		return bitangents;
	}
}
