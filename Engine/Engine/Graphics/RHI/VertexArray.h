#pragma once

// Engine Includes.
#include "Buffer.h"
#include "VertexLayout.hpp"
#include "ID/VertexArrayID.h"

// std Includes.
#include <optional>

namespace Kakadu::RHI
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

		RHI::VertexArrayID Id()	const { return id;				}
		u32 VertexCount()		const { return vertex_count;	}
		u32 IndexCount()		const { return index_count;		}
		u32 InstanceCount()		const { return instance_count;  }

	private:

		void Delete();

		void CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const VertexLayout& vertex_layout );
		void CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const Buffer& instance_buffer, const VertexLayout& vertex_layout );

	private:
		RHI::VertexArrayID id;
		/* 4 bytes of padding. */

		std::string name;

		RHI::BufferID vertex_buffer_id;
		RHI::BufferID index_buffer_id;
		RHI::BufferID instance_buffer_id;

		u32 vertex_count;
		u32 index_count;

		u32 instance_count;
	};
}
