#pragma once

// Engine Includes.
#include "Color.hpp"
#include "MeshUtility.hpp"
#include "VertexArray.h"

// std Includes.
#include <array>

namespace Engine
{
	class Mesh
	{
	public:
		enum class PrimitiveType
		{
			Points			= GL_POINTS,
			Lines			= GL_LINES,
			Line_loop		= GL_LINE_LOOP,
			Line_strip		= GL_LINE_STRIP,
			Triangles		= GL_TRIANGLES,
			Triangle_strip	= GL_TRIANGLE_STRIP,
			Triangle_fan	= GL_TRIANGLE_FAN,
			Quads			= GL_QUADS
		};

	public:
		Mesh();

		DELETE_COPY_CONSTRUCTORS( Mesh );
		DEFAULT_MOVE_CONSTRUCTORS( Mesh );

		Mesh( const std::span< const Vector3		> positions,
			  const std::string&					  name			 = {},
			  const std::span< const Vector3		> normals		 = {},
			  const std::span< const Vector2		> uvs			 = {},
			  const std::span< const std::uint32_t	> indices		 = {},
			  const std::span< const Vector3		> tangents		 = {},
			  const PrimitiveType					  primitive_type = PrimitiveType::Triangles,
			  const GLenum							  usage			 = GL_STATIC_DRAW );

		Mesh( std::vector< Vector3			>&& positions,
			  const std::string&				name		   = {},
			  std::vector< Vector3			>&& normals		   = {},
			  std::vector< Vector2			>&& uvs			   = {},
			  std::vector< std::uint32_t	>&& indices		   = {},
			  std::vector< Vector3			>&& tangents	   = {},
			  const PrimitiveType				primitive_type = PrimitiveType::Triangles,
			  const GLenum						usage		   = GL_STATIC_DRAW );

		Mesh( const Mesh& other,
			  const std::initializer_list< VertexInstanceAttribute > instanced_attributes,
			  const std::vector< float >& instance_data,
			  const int instance_count,
			  const GLenum usage = GL_STATIC_DRAW );

		~Mesh();

	/*
	 * Usage:
	 */

		void Bind() const { vertex_array.Bind(); }
		void Upload( const void* data ) const;
		void Upload_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const;
		void UpdateInstanceData( const void* data ) const;
		void UpdateInstanceData_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const;

		template< typename InstanceDataType >
		void UpdateInstanceData_Partial( const std::span< InstanceDataType > data_span, const std::size_t offset_from_buffer_start ) const
		{
			ASSERT_DEBUG_ONLY( instance_buffer && "UpdateInstanceData_Partial< T >() called on non-instanced Mesh!" );

			instance_buffer->Upload_Partial( std::as_writable_bytes( data_span ), offset_from_buffer_start );
		}

	/*
	 * Queries:
	 */

		const std::string& Name() const { return name; }

		PrimitiveType Primitive() const { return primitive_type; }

		int VertexCount() const { return vertex_buffer.Count(); }
		int IndexCount()  const { return index_buffer.has_value() ? index_buffer->Count() : 0; }

		bool HasIndices() const { return IndexCount(); }

		bool HasInstancing() const { return ( bool )instance_buffer; }
		int InstanceCount()  const { return instance_count; }

		bool IsCompatibleWith( const VertexLayout& other_vertex_layout ) const { return vertex_layout.IsCompatibleWith( other_vertex_layout ); }

	/*
	 * Index Data:
	 */

		const std::vector< std::uint32_t >&	Indices()		const { return indices; };
		const std::uint32_t*				Indices_Raw()	const { return indices.data(); };
		constexpr GLenum					IndexType()		const { return GL_UNSIGNED_INT; }

	/*
	 * Vertex Data:
	 */

		const std::vector< Vector3 >& Positions()	const { return positions;	};
		const std::vector< Vector3 >& Normals()		const { return normals;		};
		const std::vector< Vector3 >& Tangents()	const { return tangents;	};
		const std::vector< Vector2 >& Uvs()			const { return uvs;		};

		const float* Positions_Raw()	const { return reinterpret_cast< const float* >( positions.data()	); };
		const float* Normals_Raw()		const { return reinterpret_cast< const float* >( normals.data()		); };
		const float* Tangents_Raw()		const { return reinterpret_cast< const float* >( tangents.data()	); };
		const float* Uvs_Raw()			const { return reinterpret_cast< const float* >( uvs.data()			); };

	private:
		static std::array< VertexAttribute, 4 > GatherAttributes( const std::span< const Vector3 >& positions,
																  const std::span< const Vector3 >& normals,
																  const std::span< const Vector2 >& uvs,
																  const std::span< const Vector3 >& tangents );

 	private:
		std::string name;

		std::vector< std::uint32_t > indices;

		std::vector< Vector3 > positions;
		std::vector< Vector3 > normals;
		std::vector< Vector3 > tangents;
		std::vector< Vector2 > uvs;

		PrimitiveType primitive_type;

		int instance_count;

		VertexBuffer vertex_buffer;
		VertexLayout vertex_layout;
		std::optional< IndexBuffer > index_buffer;
		std::optional< InstanceBuffer > instance_buffer;
		VertexArray vertex_array;
	};
}