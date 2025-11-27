#pragma once

// Engine Includes.
#include "Buffer.hpp"
#include "VertexLayout.hpp"

// std Includes.
#include <optional>

namespace Engine
{
	class VertexArray
	{
	public:
		using ID = ID< VertexArray >;

	public:
		VertexArray( const std::string& name = {} );

		DELETE_COPY_CONSTRUCTORS( VertexArray );
		
		VertexArray( VertexArray&& donor );
		VertexArray& operator =( VertexArray&& donor );

		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, const std::string& name = {} );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout, 
					 const std::optional< IndexBuffer >& index_buffer,
					 const std::string& name = {} );
		VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout,
					 const std::optional< IndexBuffer >& index_buffer,
					 const InstanceBuffer& instance_buffer,
					 const std::string& name = {} );
		~VertexArray();

	/* Usage: */

		void Bind() const;
		void Unbind() const;

	/* Queries: */

		inline bool IsValid() const { return id.IsValid(); }

		inline ID			Id()			const { return id;				}
		inline unsigned int VertexCount()	const { return vertex_count;	}
		inline unsigned int IndexCount()	const { return index_count;		}
		inline unsigned int InstanceCount()	const { return instance_count;  }

	private:

		void Delete();

		void CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout );
		void CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const InstanceBuffer& instance_buffer, const VertexLayout& vertex_layout );

	private:
		ID id;
		//int padding;

		std::string name;

		VertexBuffer::ID vertex_buffer_id;
		IndexBuffer::ID index_buffer_id;

		InstanceBuffer::ID instance_buffer_id;

		unsigned int vertex_count;
		unsigned int index_count;

		unsigned int instance_count;
	};
}
