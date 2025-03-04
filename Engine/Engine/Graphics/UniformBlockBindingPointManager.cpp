// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engine Includes.
#include "Shader.hpp"
#include "UniformBlockBindingPointManager.h"

namespace Engine
{
	Uniform::BindingPoint UniformBlockBindingPointManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BufferInformation& uniform_buffer_info )
	{
		auto& instance = Instance();

		switch( uniform_buffer_info.category )
		{
			case Uniform::BufferCategory::Global:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_global );
			case Uniform::BufferCategory::Intrinsic:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_intrinsic );
			// case Uniform::BufferCategory::Regular:
			default:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_regular );
		}
	}

	void UniformBlockBindingPointManager::ConnectBufferToBlock( const UniformBuffer& uniform_buffer, const std::string& block_name, const Uniform::BufferCategory category )
	{
		auto& instance = Instance();

		std::optional< Uniform::BindingPoint > binding_point;

		switch( category )
		{
			case Uniform::BufferCategory::Regular:
				binding_point = instance.binding_point_book_keeping_regular.Find( block_name );
				break;
			case Uniform::BufferCategory::Global:
				binding_point = instance.binding_point_book_keeping_global.Find( block_name );
				break;
			case Uniform::BufferCategory::Intrinsic:
				binding_point = instance.binding_point_book_keeping_intrinsic.Find( block_name );
				break;
		}

		if( binding_point.has_value() )
		{
			BindBufferToBindingPoint( uniform_buffer, *binding_point );
		}
	#ifdef _DEBUG
		else
		{
		#if defined( _WIN32 )
			if( IsDebuggerPresent() )
				OutputDebugStringA( ( "\nUniformBlockBindingPointManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered.\n" ).c_str() );
		#endif // _WIN32
			throw std::runtime_error( "UniformBlockBindingPointManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered." );
		}
	#endif // _DEBUG
	}

/*
 *
 *	PRIVATE API:
 *
 */

	UniformBlockBindingPointManager::UniformBlockBindingPointManager()
		:
		/* Divide max. binding points = max. uniform buffers/blocks allowed into 3 categories and determine their starting offsets from 0. */
		binding_point_max_count( QueryMaximumUniformBufferBindingCount() ),
		binding_point_book_keeping_intrinsic( 0, 4 ),
		binding_point_book_keeping_global( 0 + 4, 4 ),
		binding_point_book_keeping_regular( 0 + 4 + 4, binding_point_max_count - 8 )
	{
	}

	Uniform::BindingPoint UniformBlockBindingPointManager::RegisterUniformBlock( const Shader& shader, const std::string& block_name, 
																				 Uniform::BindingPointBookKeeping& binding_point_book_keeping )
	{
		if( const auto maybe_binding_point = binding_point_book_keeping.Find( block_name );
			maybe_binding_point.has_value() )
		{
			const auto binding_point_found = *maybe_binding_point;

			const unsigned int block_index = glGetUniformBlockIndex( shader.Id().Get(), block_name.c_str() );
			glUniformBlockBinding( shader.Id().Get(), block_index, binding_point_found );

			return binding_point_found;
		}
		else
		{
			if( binding_point_book_keeping.HasRoom() )
			{
				const auto binding_point_to_assign = binding_point_book_keeping.Assign( block_name );

				const unsigned int block_index = glGetUniformBlockIndex( shader.Id().Get(), block_name.c_str() );
				glUniformBlockBinding( shader.Id().Get(), block_index, binding_point_to_assign );

				return binding_point_to_assign;
			}

			throw std::runtime_error( "UniformBlockBindingPointManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers." );
#if defined( _WIN32 )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "\nUniformBlockBindingPointManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers.\n" );
#endif // _WIN32
		}
	}

	void UniformBlockBindingPointManager::BindBufferToBindingPoint( const UniformBuffer& uniform_buffer, const Uniform::BindingPoint binding_point )
	{
		glBindBufferBase( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id().Get() );
	}

	void UniformBlockBindingPointManager::BindBufferToBindingPoint_Partial( const UniformBuffer& uniform_buffer, const Uniform::BindingPoint binding_point,
																			 const unsigned int offset, const unsigned int size )
	{
		glBindBufferRange( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.Id().Get(), ( GLintptr )offset, ( GLsizeiptr )size );
	}

	unsigned int UniformBlockBindingPointManager::QueryMaximumUniformBufferBindingCount()
	{
		unsigned int query_result;
		glGetIntegerv( GL_MAX_UNIFORM_BUFFER_BINDINGS, ( int* )&query_result );
		return query_result;
	}
}
