// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engine Includes.
#include "Capabilities.h"
#include "Shader.hpp"
#include "UniformBlockBindingPointManager.h"

namespace Kakadu::RHI::Uniform
{
	BindingPoint BlockBindingPointManager::RegisterUniformBlock( const Shader& shader,
																 const std::string& block_name,
																 BufferInformation& uniform_buffer_info )
	{
		auto& instance = Instance();

		switch( uniform_buffer_info.category )
		{
			case BufferCategory::Global:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_global );
			case BufferCategory::Intrinsic:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_intrinsic );
			// case BufferCategory::Regular:
			default:
				return uniform_buffer_info.binding_point = RegisterUniformBlock( shader, block_name, instance.binding_point_book_keeping_regular );
		}
	}

	void BlockBindingPointManager::ConnectBufferToBlock( const Buffer& uniform_buffer,
														 const std::string& block_name,
														 const BufferCategory category )
	{
		auto& instance = Instance();

		std::optional< BindingPoint > binding_point;

		switch( category )
		{
			case BufferCategory::Regular:
				binding_point = instance.binding_point_book_keeping_regular.Find( block_name );
				break;
			case BufferCategory::Global:
				binding_point = instance.binding_point_book_keeping_global.Find( block_name );
				break;
			case BufferCategory::Intrinsic:
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
				OutputDebugStringA( ( "\nBlockBindingPointManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered.\n" ).c_str() );
		#endif // _WIN32
			throw std::runtime_error( "BlockBindingPointManager::ConnectBufferToBlock(): Block with name \"" + block_name + "\" was not registered." );
		}
	#endif // _DEBUG
	}

/*
 *
 *	PRIVATE API:
 *
 */

	BlockBindingPointManager::BlockBindingPointManager()
		:
		/* Divide max. binding points = max. uniform buffers/blocks allowed into 3 categories and determine their starting offsets from 0. */
		binding_point_max_count( RHI::Capabilities::QueryMaximumUniformBufferBindingCount() ),
		binding_point_book_keeping_intrinsic( 0, 4 ),
		binding_point_book_keeping_global( 0 + 4, 4 ),
		binding_point_book_keeping_regular( 0 + 4 + 4, binding_point_max_count - 8 )
	{
	}

	BindingPoint BlockBindingPointManager::RegisterUniformBlock( const Shader& shader,
																 const std::string& block_name, 
																 BlockBindingPointBookKeeping& binding_point_book_keeping )
	{
		if( const auto maybe_binding_point = binding_point_book_keeping.Find( block_name );
			maybe_binding_point.has_value() )
		{
			const auto binding_point_found = *maybe_binding_point;

			const u32 block_index = glGetUniformBlockIndex( shader.Id().id, block_name.c_str() );
			glUniformBlockBinding( shader.Id().id, block_index, binding_point_found );

			return binding_point_found;
		}
		else
		{
			if( binding_point_book_keeping.HasRoom() )
			{
				const auto binding_point_to_assign = binding_point_book_keeping.Assign( block_name );

				const u32 block_index = glGetUniformBlockIndex( shader.Id().id, block_name.c_str() );
				glUniformBlockBinding( shader.Id().id, block_index, binding_point_to_assign );

				return binding_point_to_assign;
			}

			throw std::runtime_error( "BlockBindingPointManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers." );
#if defined( _WIN32 )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "\nBlockBindingPointManager::RegisterUniformBuffer(): Maximum binding point count has been reached. Can not assign new blocks/buffers.\n" );
#endif // _WIN32
		}
	}

	void BlockBindingPointManager::BindBufferToBindingPoint( const Buffer& uniform_buffer, 
															 const BindingPoint binding_point )
	{
		glBindBufferBase( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.id.id );
	}

	void BlockBindingPointManager::BindBufferToBindingPoint_Partial( const Buffer& uniform_buffer,
																	 const BindingPoint binding_point,
																	 const u32 offset,
																	 const u32 size )
	{
		glBindBufferRange( GL_UNIFORM_BUFFER, binding_point, uniform_buffer.id.id, ( GLintptr )offset, ( GLsizeiptr )size );
	}
}
