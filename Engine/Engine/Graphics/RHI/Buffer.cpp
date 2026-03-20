// Engine Includes.
#include "RHI.h"
#include "Buffer.h"
#include "Graphics/GLLogger.h" // TODO: GLLogger dependency - wrong direction, fix when logger is properly split.
#include "GLLabelPrefixes.h"
#include "Core/Assertion.h"
#include "Core/Optimization.h"
#include "Core/ServiceLocator.h"

// std Includes.
#include <map>

namespace Kakadu::RHI
{
	/*
	 * Internal variables:
	 */

	internal_variable std::map< BufferID, u32 > REF_COUNT_MAP;

	/*
	 * Internal functions:
	 */

	internal_function constexpr GLenum TypeToGLEnum( BufferType buffer_type )
	{
		switch( buffer_type )
		{
			case BufferType::Vertex:	return GL_ARRAY_BUFFER;
			case BufferType::Index:		return GL_ELEMENT_ARRAY_BUFFER;
			case BufferType::Instance:	return GL_ARRAY_BUFFER;
			case BufferType::Uniform:	return GL_UNIFORM_BUFFER;

			case BufferType::Invalid:
				ASSERT( false && "Invalid buffer_type in TypeToGLEnum( BufferType )!" );
				return GL_NONE;
		}

		UNREACHABLE();
	}

	internal_function void Create( Buffer& buffer, const void* data, const Usage usage )
	{
		glGenBuffers( 1, &buffer.id.id );
		REF_COUNT_MAP[ buffer.id ]++;

		buffer.Bind();
		glBufferData( TypeToGLEnum( buffer.type ), buffer.size, data, UsageToGLEnum( usage ) );
	}

	internal_function void CloneBuffer( Buffer& buffer )
	{
		REF_COUNT_MAP[ buffer.id ]++;
	}

	internal_function void Delete( Buffer& buffer )
	{
		if( buffer && --REF_COUNT_MAP[ buffer.id ] == 0 )
		{
#ifdef _EDITOR
			switch( buffer.type )
			{
				case BufferType::Vertex:	std::cout << "Deleting Vertex Buffer id #";		break;
				case BufferType::Instance:	std::cout << "Deleting Instance Buffer id #";	break;
				case BufferType::Index:		std::cout << "Deleting Index Buffer id #";		break;
				case BufferType::Uniform:	std::cout << "Deleting Uniform Buffer id #";	break;

				case BufferType::Invalid:
					std::cerr << "Attempting to delete an invalid buffer!";
					ASSERT( false && "Attempting to delete an invalid buffer!" );
					return;
			}

			std::cout << buffer.id.id << ".\n";
#endif // _EDITOR

			glDeleteBuffers( 1, &buffer.id.id );
			REF_COUNT_MAP.erase( buffer.id );
			buffer.id.Reset(); // OpenGL does not reset the ID to zero.
		}
	}

#ifdef _EDITOR
	internal_function constexpr const char* LabelPrefix( const BufferType type )
	{
		switch( type )
		{
			case BufferType::Vertex:	return GL_LABEL_PREFIX_VERTEX_BUFFER;
			case BufferType::Index:		return GL_LABEL_PREFIX_INDEX_BUFFER;
			case BufferType::Instance:	return GL_LABEL_PREFIX_INSTANCE_BUFFER;
			case BufferType::Uniform:	return GL_LABEL_PREFIX_UNIFORM_BUFFER;

			case BufferType::Invalid:
				std::cerr << "LabelPrefix( type ) is called with an invalid buffer!";
				ASSERT( false && "LabelPrefix( type ) is called with an invalid buffer!" );
				return GL_LABEL_PREFIX_UNKNOWN;
		}

		UNREACHABLE();
	}
#endif // EDITOR

	Buffer::Buffer()
		:
		id(),
		type(),
		name(),
		count( 0 ),
		size( 0 )
	{
	}

	/* Only allocates memory.*/
	Buffer::Buffer( const BufferType type,
					const u32 size,
					const std::string& name,
					const Usage usage )
		:
		id(),
		type( type ),
		name( name ),
		count( 0 ),
		size( size )
	{
		ASSERT_DEBUG_ONLY( size > 0 && "'size' parameter passed to Buffer::Buffer( const BufferType type, const u32 size, const std::string& name, const Usage usage ) is empty!" );

		Create( *this, nullptr, usage );

#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.id, LabelPrefix( type ) + name );
#endif // _EDITOR
	}

	/* Allocates and sets buffer memory. */
	Buffer::Buffer( const BufferType type,
					const u32 count,
					const std::span< const std::byte > data_span,
					const std::string& name,
					const Usage usage )
		:
		id(),
		type( type ),
		name( name ),
		count( count ),
		size( ( u32 )data_span.size_bytes() )
	{
		ASSERT_DEBUG_ONLY( size > 0 && count > 0 && "'data_span' parameter passed to "
							"Buffer::Buffer( const BufferType type, const u32 count, const std::span< const std::byte > data_span, const std::string& name, const Usage usage )"
							"is empty!" );

		Create( *this, data_span.data(), usage );

#ifdef _EDITOR
		if( not name.empty() )
			ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.id, LabelPrefix( type ) + name );
#endif // _EDITOR
	}

	Buffer::Buffer( const Buffer& other )
		:
		id( other.id ),
		type( other.type ),
		name( other.name ),
		count( other.count ),
		size( other.size )
	{
		CloneBuffer( *this );
	}

	Buffer& Buffer::operator=( const Buffer& other )
	{
		id    = other.id;
		type  = other.type;
		name  = other.name;
		count = other.count;
		size  = other.size;

		CloneBuffer( *this );

		return *this;
	}

	Buffer::Buffer( Buffer&& donor )
		:
		id( std::exchange( donor.id, {} ) ),
		type( std::exchange( donor.type, {} ) ),
		name( std::exchange( donor.name, {} ) ),
		count( std::exchange( donor.count, 0 ) ),
		size( std::exchange( donor.size, 0 ) )
	{
	}
		
	Buffer& Buffer::operator=( Buffer&& donor )
	{
		Delete( *this );

		id    = std::exchange( donor.id,	{} );
		type  = std::exchange( donor.type,	{} );
		name  = std::exchange( donor.name,	{} );
		count = std::exchange( donor.count,  0 );
		size  = std::exchange( donor.size,	 0 );

		return *this;
	}
		
	Buffer::~Buffer()
	{
		Delete( *this );
	}

/* Usage: */

	void Buffer::Bind() const
	{
		ASSERT_DEBUG_ONLY( id && "Attempting Bind() on Buffer with zero size!" );

		glBindBuffer( TypeToGLEnum( type ), id.id );
	}

	void Buffer::Upload( const void* data ) const
	{
		Bind();
		glBufferSubData( TypeToGLEnum( type ), 0, size, data );
	}

	void Buffer::Upload_Partial( const std::span< const std::byte > data_span, const std::size_t offset_from_buffer_start ) const
	{
		Bind();
		glBufferSubData( TypeToGLEnum( type ), ( GLintptr )offset_from_buffer_start, ( GLsizeiptr )data_span.size_bytes(), ( void* )data_span.data() );
	}
}
