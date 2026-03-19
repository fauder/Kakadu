#pragma once

// Engine Includes.
#include "Buffer.h"
#include "Uniform.h"

// std Includes.
#include <optional>

namespace Kakadu
{
	/* Forward declaration: */
	class Shader;

	// Singleton.
	class UniformBlockBindingPointManager
	{
	public:
		DELETE_COPY_AND_MOVE_CONSTRUCTORS( UniformBlockBindingPointManager );

	/* Block registering API: */
		static Uniform::BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BufferInformation& uniform_buffer_info );

	/* Buffer-Block connecting API: */
		static void ConnectBufferToBlock( const Buffer& uniform_buffer, const std::string& block_name, const Uniform::BufferCategory category = Uniform::BufferCategory::Regular );

	private:
	/* Private default constructor. */
		UniformBlockBindingPointManager();

	/* Singleton related: */
		static UniformBlockBindingPointManager& Instance()
		{
			local_persist UniformBlockBindingPointManager instance;
			return instance;
		}

	/* Block registering API: */
		static Uniform::BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, Uniform::BindingPointBookKeeping& binding_point_book_keeping );

	/* Buffer-Block connecting API: */
		static void BindBufferToBindingPoint( const Buffer&, const Uniform::BindingPoint );
		static void BindBufferToBindingPoint_Partial( const Buffer&, const Uniform::BindingPoint,
													  const u32 offset, const u32 size );

	/* Misc.: */
		static u32 QueryMaximumUniformBufferBindingCount();

	private:
		u32 binding_point_max_count;
		//int padding;

		Uniform::BindingPointBookKeeping binding_point_book_keeping_intrinsic;
		Uniform::BindingPointBookKeeping binding_point_book_keeping_global;
		Uniform::BindingPointBookKeeping binding_point_book_keeping_regular;
	};
}