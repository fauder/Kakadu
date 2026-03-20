// Engine Includes.
#include "RHI.h"
#include "GLLabelPrefixes.h"
#include "VertexArray.h"
#include "Graphics/GLLogger.h" // TODO: GLLogger dependency - wrong direction, fix when logger is properly split.
#include "Core/ServiceLocator.h"

// std Includes.
#include <utility>

namespace Kakadu::RHI
{
	VertexArray::VertexArray( const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( {} ),
		index_buffer_id( {} ),
		instance_buffer_id( {} ),
		vertex_count( 0 ),
		index_count( 0 ),
		instance_count( 0 )
	{
	}

	VertexArray::VertexArray( VertexArray&& donor )
		:
		name( std::exchange( donor.name, {} ) ),
		vertex_buffer_id( std::exchange( donor.vertex_buffer_id, {} ) ),
		index_buffer_id( std::exchange( donor.index_buffer_id, {} ) ),
		instance_buffer_id( std::exchange( donor.instance_buffer_id, {} ) ),
		vertex_count( std::exchange( donor.vertex_count, 0 ) ),
		index_count( std::exchange( donor.index_count, 0 ) ),
		instance_count( std::exchange( donor.instance_count, 0 ) )
	{
		Delete();

		id = std::exchange( donor.id, {} );
	}

	VertexArray& VertexArray::operator=( VertexArray&& donor )
	{
		Delete();

		id                  = std::exchange( donor.id,					{} );
		name                = std::exchange( donor.name,				{} );
		vertex_buffer_id    = std::exchange( donor.vertex_buffer_id,	{} );
		index_buffer_id     = std::exchange( donor.index_buffer_id,		{} );
		instance_buffer_id  = std::exchange( donor.instance_buffer_id,	{} );
		vertex_count        = std::exchange( donor.vertex_count,		0 );
		index_count         = std::exchange( donor.index_count,			0 );
		instance_count      = std::exchange( donor.instance_count,		0 );

		return *this;
	}

	VertexArray::VertexArray( const Buffer& vertex_buffer,
							  const VertexLayout& vertex_buffer_layout, 
							  const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( vertex_buffer.id ),
		index_buffer_id( {} ),
		instance_buffer_id( {} ),
		vertex_count( vertex_buffer.count ),
		index_count( 0 ),
		instance_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const Buffer& vertex_buffer, 
							  const VertexLayout& vertex_buffer_layout, 
							  const std::optional< Buffer >& index_buffer,
							  const std::string& name )
		:
		id( -1 ),
		name( name ),
		vertex_buffer_id( vertex_buffer.id ),
		index_buffer_id( index_buffer
						 ? index_buffer->id
						 : RHI::BufferID{} ),
		instance_buffer_id( -1 ),
		vertex_count( vertex_buffer.count ),
		index_count( index_buffer
					 ? index_buffer->count
					 : 0 ),
		instance_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );
		if( index_buffer )
			index_buffer->Bind();

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const Buffer& vertex_buffer, const VertexLayout& vertex_buffer_layout,
							  const std::optional< Buffer >& index_buffer,
							  const Buffer& instance_buffer,
							  const std::string& name )
		:
		id( -1 ),
		name( name ),
		vertex_buffer_id( vertex_buffer.id ),
		index_buffer_id( index_buffer
							? index_buffer->id
							: RHI::BufferID{} ),
		instance_buffer_id( instance_buffer.id ),
		vertex_count( vertex_buffer.count ),
		index_count( index_buffer
						? index_buffer->count
						: 0 ),
		instance_count( instance_buffer.count )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, instance_buffer, vertex_buffer_layout );

		if( index_buffer )
			index_buffer->Bind();

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::~VertexArray()
	{
		Delete();
	}

	void VertexArray::Bind() const
	{
		glBindVertexArray( id.id );
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray( 0 );
	}

	void VertexArray::Delete()
	{
		if( IsValid() )
		{
			glDeleteVertexArrays( 1, &id.id );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const VertexLayout& vertex_layout )
	{
		glGenVertexArrays( 1, &id.id );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_VERTEX_ARRAY, id.id, GL_LABEL_PREFIX_VERTEX_ARRAY + name );
	#endif // _EDITOR

		vertex_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_NonInstanced();
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const Buffer& vertex_buffer, const Buffer& instance_buffer, const VertexLayout& vertex_layout )
	{
		glGenVertexArrays( 1, &id.id );
		Bind();

	#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_VERTEX_ARRAY, id.id, GL_LABEL_PREFIX_VERTEX_ARRAY + name );
	#endif // _EDITOR

		vertex_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_NonInstanced();
		instance_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_Instanced();
	}
}