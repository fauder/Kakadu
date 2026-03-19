#pragma once

// Engine Includes.
#include "Buffer.h"
#include "VertexLayout.hpp"
#include "RHI/ID/VertexArrayID.h"

// std Includes.
#include <optional>

namespace Kakadu
{
	class VertexArray
	{
	public:
		VertexArray( const std::string& name = {} );

		DELETE_COPY_CONSTRUCTORS( VertexArray );
		
		VertexArray( VertexArray&& donor );
		VertexArray& operator =( VertexArray&& donor );

		VertexArray( const Buffer& vertex_buffer, const VertexLayout& vertex_layout, const std::string& name = {} );
		VertexArray( const Buffer& vertex_buffer, const VertexLayout& vertex_layout, 
					 const std::optional< Buffer >& index_buffer,
					 const std::string& name = {} );
		VertexArray( const Buffer& vertex_buffer, const VertexLayout& vertex_layout,
					 const std::optional< Buffer >& index_buffer,
					 const Buffer& instance_buffer,
					 const std::string& name = {} );
		~VertexArray();

	/* Usage: */

		void Bind() const;
		void Unbind() const;

	/* Queries: */

		bool IsValid() const { return ( bool )id; }

		RHI::VertexArrayID Id()			const { return id;				}
		unsigned int VertexCount()		const { return vertex_count;	}
		unsigned int IndexCount()		const { return index_count;		}
		unsigned int InstanceCount()	const { return instance_count;  }

	private:

		void Delete();

		void CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const VertexLayout& vertex_layout );
		void CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const Buffer& instance_buffer, const VertexLayout& vertex_layout );

	private:
		RHI::VertexArrayID id;
		//int padding;

		std::string name;

		RHI::BufferID vertex_buffer_id;
		RHI::BufferID index_buffer_id;
		RHI::BufferID instance_buffer_id;

		unsigned int vertex_count;
		unsigned int index_count;

		unsigned int instance_count;
	};
}
