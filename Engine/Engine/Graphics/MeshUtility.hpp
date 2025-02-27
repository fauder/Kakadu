#pragma once

// Engine Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <utility>
#include <vector>

namespace Engine
{
	namespace MeshUtility
	{
		template< typename VertexAttributeArray, typename ... VertexAttributeArrays >
		constexpr auto Interleave( const VertexAttributeArray& vertex_attribute_array_first, const VertexAttributeArrays& ... vertex_attribute_array_pack )
		{
			// TODO: ASSERT that the array sizes are the same.
			// TODO: ASSERT that the array element types are the same.

			constexpr std::size_t attribute_count_first = std::remove_reference_t< decltype( vertex_attribute_array_first[ 0 ] ) >::Dimension();
			constexpr std::size_t attribute_count       = attribute_count_first + ( std::remove_reference_t< decltype( vertex_attribute_array_pack[ 0 ] ) >::Dimension() + ... );

			constexpr std::size_t vertex_count = std::tuple_size< std::remove_reference_t< decltype( vertex_attribute_array_first ) > >::value;

			using ComponentType = std::remove_reference_t< decltype( vertex_attribute_array_first[ 0 ] ) >::ComponentType;

			std::array< Math::Vector< ComponentType, attribute_count >, vertex_count > interleaved_vertex_attribute_array;

			for( std::size_t vertex_index = 0; vertex_index < vertex_count; vertex_index++ )
			{
				for( std::size_t attribute_index = 0; attribute_index < attribute_count; )
				{
					/* Below for loop covers the assignment of the elements of the FIRST attribute array. */
					for( std::size_t component_index = 0; component_index < attribute_count_first; component_index++ )
						interleaved_vertex_attribute_array[ vertex_index ][ attribute_index++ ] = vertex_attribute_array_first[ vertex_index ][ component_index ];

					/* Below fold expression of the lambda over the comma operator covers the assignment of the rest; i.e., array elements in the attribute array PACK. */
					( [ & ]
					{
						if( !vertex_attribute_array_pack.empty() )
						{
							constexpr std::size_t dimension = std::remove_reference_t< decltype( vertex_attribute_array_pack[ 0 ] ) >::Dimension();
							for( std::size_t component_index = 0; component_index < dimension; component_index++ )
								interleaved_vertex_attribute_array[ vertex_index ][ attribute_index++ ] = vertex_attribute_array_pack[ vertex_index ][ component_index ];
						}
					}() , ... ); // Fold expression: Lambda over the comma operator.
				}
			}

			return interleaved_vertex_attribute_array;
		}

		template< typename T >
		constexpr unsigned int ComponentCount()
		{
			if constexpr( Concepts::IsVector< T > )
				return ( unsigned int )T::Dimension();
			else
				return 1;
		}

		template< typename FirstVertexAttributeType, typename ... OtherVertexAttributeTypes >
		auto Interleave( unsigned int& vertex_count,
						 FirstVertexAttributeType&& vertex_attribute_vector_first,
						 OtherVertexAttributeTypes&& ... vertex_attribute_vector_pack )
		{
			auto ComponentCount_OnlyForNonEmpty = []( auto&& vector_of_attributes )
			{
				if( vector_of_attributes.empty() )
					return 0u;

				return ComponentCount< typename std::remove_reference_t< decltype( vector_of_attributes ) >::value_type >();
			};

			constexpr auto first_attribute_component_count = ComponentCount< typename std::remove_reference_t< decltype( vertex_attribute_vector_first ) >::value_type >();
			const	  auto total_attribute_count           = first_attribute_component_count +
														     ( ComponentCount_OnlyForNonEmpty( vertex_attribute_vector_pack ) + ... );
			vertex_count = ( unsigned int )vertex_attribute_vector_first.size();

			std::vector< float > interleaved_vertex_attribute_vector( total_attribute_count * vertex_count );

			constexpr auto size_of_first_vertex_attribute = sizeof( std::remove_reference_t< decltype( vertex_attribute_vector_first ) >::value_type );

			for( unsigned int vertex_index = 0, element_index = 0; vertex_index < vertex_count; vertex_index++ )
			{
				/* Copy the first attribute's elements. */
				std::memcpy( interleaved_vertex_attribute_vector.data() + element_index, 
							 vertex_attribute_vector_first.data() + vertex_index,
							 size_of_first_vertex_attribute );
				element_index += first_attribute_component_count;

				/* Below fold expression of the lambda over the comma operator covers the copying of the rest; i.e., elements in the parameter pack. */
				( [&]
				{
					if( !vertex_attribute_vector_pack.empty() )
					{
						const auto size_of_vertex_attribute = sizeof( std::remove_reference_t< decltype( vertex_attribute_vector_pack ) >::value_type );
						const auto component_count_of_vertex_attribute = ComponentCount_OnlyForNonEmpty( vertex_attribute_vector_pack );
						std::memcpy( interleaved_vertex_attribute_vector.data() + element_index,
									 vertex_attribute_vector_pack.data() + vertex_index,
									 size_of_vertex_attribute );
						element_index += component_count_of_vertex_attribute;
					}
				}() , ... ); // Fold expression: Lambda over the comma operator.
			}

			return interleaved_vertex_attribute_vector;
		}
	}
}
