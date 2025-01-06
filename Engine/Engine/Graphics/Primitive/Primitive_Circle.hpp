#pragma once

// Project Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <array>
#include <functional>

namespace Engine::Primitive::Indexed::CircleTemplate
{
	template< std::uint8_t VertexCount = 20, float Diameter = 1.0f > requires( VertexCount >= 3 )
	std::array< Vector3, VertexCount > Positions()
	{
		std::array< Vector3, VertexCount > positions;

		constexpr float radius = Diameter / 2.0f;
		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / VertexCount;

		for( std::uint8_t i = 0; i < VertexCount; i++ )
			positions[ i ] = Vector3( radius * Math::Cos( delta_angle * i ), 0.0f, radius * -Math::Sin( delta_angle * i ) );

		return positions;
	};

	template< std::uint8_t VertexCount = 20 > requires( VertexCount >= 3 )
	std::array< unsigned int, 3 * ( VertexCount - 2 ) > Indices()
	{
		using IndexType = unsigned int;

		constexpr IndexType triangle_count = VertexCount - 2;
		constexpr IndexType index_count    = triangle_count * 3;
		std::array< IndexType, index_count > indices;

		IndexType array_index = 0;

		auto FindTriangleMidpointIndex = []( const IndexType start_index, const IndexType end_index )
		{
			return start_index + IndexType( Math::Round( ( end_index - start_index ) / 2.0f ) );
		};

		/* Assigns the indices of the largest triangle inside the range it is given and then recursively calls itself to triangulate the (up to) 2 smaller triangular areas.
		 * A potential 3rd area is not included in this routine, because that area can only form for the special case of the initial triangulation of a circle,
		 * which is handled explicitly outside this routine. */
		std::function< void( const IndexType, const IndexType ) > Triangulate = [ & ]( const IndexType start_index, const IndexType end_index )
		{
			const IndexType effective_end_index = end_index + VertexCount * ( end_index == 0 );

			const auto midpoint_index = FindTriangleMidpointIndex( start_index, effective_end_index );

			indices[ array_index++ ] = start_index;
			indices[ array_index++ ] = midpoint_index;
			indices[ array_index++ ] = end_index;

			if( start_index < ( midpoint_index - 1 ) )
				Triangulate( start_index, midpoint_index );

			if( midpoint_index < ( effective_end_index - 1 ) )
				Triangulate( midpoint_index, end_index );
		};

		/* The largest triangle is determined by finding the initial delta index and then using that to find an end index.
		 * This largest triangle may create up to 3 other smaller triangles.
		 * The recursive Triangulate() lambda above finds the largest triangle for a given start-end index range and then handles up to 2 smaller areas.
		 * The 3rd area can only form during the special case of the initial triangulation.
		 * It will be explicitly handled at the end. */

		const IndexType initial_delta_index        = IndexType( Math::Round( VertexCount / 3.0f ) );
		const IndexType largest_triangle_end_index = 2 * initial_delta_index;

		Triangulate( 0, largest_triangle_end_index );

		/* Account for the 3rd possible sub-area: */
		const IndexType index_of_last_vertex = VertexCount - 1;

		if( largest_triangle_end_index < index_of_last_vertex )
			Triangulate( largest_triangle_end_index, 0 );

		return indices;
	};

	template< std::uint8_t VertexCount = 20 > requires( VertexCount >= 3 )
	std::array< Vector2, VertexCount > UVs()
	{
		std::array< Vector2, VertexCount > uvs;

		constexpr Radians delta_angle = Constants< Radians >::Two_Pi() / VertexCount;

		for( std::uint8_t i = 0; i < VertexCount; i++ )
			uvs[ i ] = Vector2( Math::Cos( delta_angle * i ), -Math::Sin( delta_angle * i ) ) * 0.5f + 0.5f; // Remap unit circle to uv range: [-1,+1] => [0,+1].

		return uvs;
	};

	template< std::uint8_t VertexCount = 20 > requires( VertexCount >= 3 )
	std::array< Vector3, VertexCount > Normals()
	{
		std::array< Vector3, VertexCount > normals;
		normals.fill( Vector3::Up() );
		return normals;
	};

	template< std::uint8_t VertexCount = 20 > requires( VertexCount >= 3 )
	std::array< Vector3, VertexCount > Tangents()
	{
		std::array< Vector3, VertexCount > tangents;
		tangents.fill( Vector3::Right() );
		return tangents;
	};

	template< std::uint8_t VertexCount = 20 > requires( VertexCount >= 3 )
	std::array< Vector3, VertexCount > Bitangents()
	{
		std::array< Vector3, VertexCount > bitangents;
		bitangents.fill( Vector3::Forward() );
		return bitangents;
	};
}
