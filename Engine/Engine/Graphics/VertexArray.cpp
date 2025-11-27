// Engine Includes.
#include "Graphics.h"
#include "VertexArray.h"

// std Includes.
#include <utility>

namespace Engine
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

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer,
							  const VertexLayout& vertex_buffer_layout, 
							  const std::string& name )
		:
		id( {} ),
		name( name ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( {} ),
		instance_buffer_id( {} ),
		vertex_count( vertex_buffer.Count() ),
		index_count( 0 ),
		instance_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, 
							  const VertexLayout& vertex_buffer_layout, 
							  const std::optional< IndexBuffer >& index_buffer,
							  const std::string& name )
		:
		id( -1 ),
		name( name ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( index_buffer
						 ? index_buffer->Id()
						 : IndexBuffer::ID{} ),
		instance_buffer_id( -1 ),
		vertex_count( vertex_buffer.Count() ),
		index_count( index_buffer
					 ? index_buffer->Count()
					 : 0 ),
		instance_count( 0 )
	{
		CreateArrayAndRegisterVertexBufferAndAttributes( vertex_buffer, vertex_buffer_layout );
		if( index_buffer )
			index_buffer->Bind();

		Unbind(); // To prevent unwanted register/unregister of buffers/layouts etc.
	}

	VertexArray::VertexArray( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_buffer_layout,
							  const std::optional< IndexBuffer >& index_buffer,
							  const VertexBuffer& instance_buffer,
							  const std::string& name )
		:
		id( -1 ),
		name( name ),
		vertex_buffer_id( vertex_buffer.Id() ),
		index_buffer_id( index_buffer
							? index_buffer->Id()
							: IndexBuffer::ID{} ),
		instance_buffer_id( instance_buffer.Id() ),
		vertex_count( vertex_buffer.Count() ),
		index_count( index_buffer
						? index_buffer->Count()
						: 0 ),
		instance_count( instance_buffer.Count() )
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
		glBindVertexArray( id.Get() );
	}

	void VertexArray::Unbind() const
	{
		glBindVertexArray( 0 );
	}

	void VertexArray::Delete()
	{
		if( IsValid() )
		{
			glDeleteVertexArrays( 1, id.Address() );
			id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const VertexLayout& vertex_layout )
	{
		glGenVertexArrays( 1, id.Address() );
		Bind();

#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_VERTEX_ARRAY, id.Get(), KAKADU_LABEL_PREFIX "VTX_ARR " +  name );
#endif // _EDITOR

		vertex_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_NonInstanced();
	}

	void VertexArray::CreateArrayAndRegisterVertexBufferAndAttributes( const VertexBuffer& vertex_buffer, const InstanceBuffer& instance_buffer, const VertexLayout& vertex_layout )
	{
		glGenVertexArrays( 1, id.Address() );
		Bind();

	#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_VERTEX_ARRAY, id.Get(), KAKADU_LABEL_PREFIX "VTX_ARR " + name );
	#endif // _EDITOR

		vertex_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_NonInstanced();
		instance_buffer.Bind();
		vertex_layout.SetAndEnableAttributes_Instanced();
	}
}