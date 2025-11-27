#pragma once

// Engine Includes.
#include "GLLogger.h"
#include "Graphics.h"
#include "ID.hpp"
#include "Core/ServiceLocator.h"
#include "Core/Assertion.h"

// std Includes.
#include <map>
#include <span>
#include <string>
#include <cstddef> // std::byte.

// TODO: Keep track of buffer ids and debug/inspect them via ImGuiDrawer::Draw() or something.

namespace Engine
{
	template< GLenum TargetType >
	class Buffer
	{
	public:
		using ID = ID< Buffer >;

	public:
		Buffer()
			:
			id( ID( 0 ) ),
			name(),
			count( 0 ),
			size( 0 )
		{
		}

		/* Only allocate memory.*/
		Buffer( const unsigned int size, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( 0 ),
			size( size )
		{
			ASSERT_DEBUG_ONLY( size > 0 && "'size' parameter passed to Buffer::Buffer( const unsigned int size, const std::string& name, const GLenum usage ) is empty!" );

			Create( nullptr, usage );

#ifdef _EDITOR
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), LabelPrefix() + name );
#endif // _EDITOR
		}

		template< typename BufferElementType >
		Buffer( const std::span< BufferElementType > data_span, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( ( unsigned int )data_span.size() ),
			size( ( unsigned int )data_span.size_bytes() )
		{
			ASSERT_DEBUG_ONLY( size > 0 && count > 0 && "'data_span' parameter passed to "
							   "Buffer::Buffer< BufferElementType >( const std::span< BufferElementType > data_span, const std::string& name, const GLenum usage ) is empty!" );

			Create( ( void* )data_span.data(), usage );

#ifdef _EDITOR
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), LabelPrefix() + name );
#endif // _EDITOR
		}

		/* To support cases where the count can not be deduced from the type alone;
		 * For example consider a vertex buffer that is passed as std::span< float >, with the following vertex attributes: 3x floats position, 3x floats normal, 2x floats uv.
		 * data_span.size() would give the actual vertex count times 8 (3 + 3 + 2) in this case. */
		template< typename BufferElementType >
		Buffer( const unsigned int count, const std::span< BufferElementType > data_span, const std::string& name = {}, const GLenum usage = GL_STATIC_DRAW )
			:
			id( ID( 0 ) ),
			name( name ),
			count( count ),
			size( ( unsigned int )data_span.size_bytes() )
		{
			ASSERT_DEBUG_ONLY( size > 0 && count > 0 && "'data_span' parameter passed to "
							   "Buffer::Buffer< BufferElementType >( const unsigned int count, const std::span< BufferElementType > data_span, const std::string& name, const GLenum usage )"
							   "is empty!" );

			Create( ( void* )data_span.data(), usage );

#ifdef _EDITOR
			if( not name.empty() )
				ServiceLocator< GLLogger >::Get().SetLabel( GL_BUFFER, id.Get(), LabelPrefix() + name );
#endif // _EDITOR
		}

		Buffer( const Buffer& other )
			:
			id( other.id ),
			name( other.name ),
			count( other.count ),
			size( other.size )
		{
			CloneBuffer();
		}

		Buffer& operator=( const Buffer& other )
		{
			id    = other.id;
			name  = other.name;
			count = other.count;
			size  = other.size;

			CloneBuffer();

			return *this;
		}

		/* Allow moving. */
		Buffer( Buffer&& donor )
			:
			name( std::exchange( donor.name, {} ) ),
			count( std::exchange( donor.count, 0 ) ),
			size( std::exchange( donor.size, 0 ) )
		{
			Delete();

			id = std::exchange( donor.id, {} );
		}
		
		Buffer& operator=( Buffer&& donor )
		{
			Delete();

			id    = std::exchange( donor.id,	{} );
			name  = std::exchange( donor.name,	{} );
			count = std::exchange( donor.count,  0 );
			size  = std::exchange( donor.size,	 0 );

			return *this;
		}
		
		~Buffer()
		{
			Delete();
		}

	/* Usage: */

		void Bind() const
		{
			ASSERT_DEBUG_ONLY( IsValid() && "Attempting Bind() on Buffer with zero size!" );

			glBindBuffer( TargetType, id.Get() );
		}

		void Upload( const void* data ) const
		{
			Bind();
			glBufferSubData( TargetType, 0, size, data );
		}

		void Upload_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
		{
			Bind();
			glBufferSubData( TargetType, ( GLintptr )offset_from_buffer_start, ( GLsizeiptr )data_span.size_bytes(), ( void* )data_span.data() );
		}

	/* Queries: */

		bool IsValid() const { return id.IsValid(); } // Use the size to implicitly define validness state.
		
		const ID&		Id()	const { return id;		}
		unsigned int	Size()	const { return size;	}
		unsigned int	Count()	const { return count;	}

	private:
		void Create( const void* data, const GLenum usage = GL_STATIC_DRAW )
		{
			glGenBuffers( 1, id.Address() );
			REF_COUNT_MAP[ id ]++;

			Bind();
			glBufferData( TargetType, size, data, usage );
		}

		void CloneBuffer()
		{
			REF_COUNT_MAP[ id ]++;
		}

		void Delete() 
		{
			if( IsValid() && --REF_COUNT_MAP[ id ] == 0 )
			{
#ifdef _EDITOR
				switch( TargetType )
				{
					case GL_ARRAY_BUFFER:			std::cout << "Deleting Vertex (or Instance) Buffer "; break;
					case GL_ELEMENT_ARRAY_BUFFER:	std::cout << "Deleting Index Buffer "; break;
					case GL_UNIFORM_BUFFER:			std::cout << "Deleting Uniform Buffer "; break;
				}

				std::cout << id.Get() << ".\n";
#endif // _EDITOR

				glDeleteBuffers( 1, id.Address() );
				REF_COUNT_MAP.erase( id );
				id.Reset(); // OpenGL does not reset the id to zero.
			}
		}

		static constexpr const char* LabelPrefix()
		{
			if constexpr( TargetType == GL_ARRAY_BUFFER )
				return "VTX_BUF ";
			else if constexpr( TargetType == GL_ELEMENT_ARRAY_BUFFER )
				return "IDX_BUF ";
			else if constexpr( TargetType == GL_UNIFORM_BUFFER )
				return "UNI_BUF ";
			else
				return "UNKNOWNBUFFERTYPE ";
		}

	private:
		ID id;
		//int padding;
		std::string name;
		unsigned int count;
		unsigned int size;

		static std::map< ID, unsigned int > REF_COUNT_MAP;
	};

	using VertexBuffer   = Buffer< GL_ARRAY_BUFFER >;
	using InstanceBuffer = Buffer< GL_ARRAY_BUFFER >;
	using IndexBuffer    = Buffer< GL_ELEMENT_ARRAY_BUFFER >;
	using UniformBuffer  = Buffer< GL_UNIFORM_BUFFER >;

	/* Static member variable definitions: */

	template< GLenum TargetType >
	std::map< typename Buffer< TargetType >::ID, unsigned int > Buffer< TargetType >::REF_COUNT_MAP;
}
