#pragma once

// Engine Includes.
#include "Buffer.h"
#include "Uniform.h"

// std Includes.
#include <optional>
#include <string>
#include <unordered_map>

// TODO: Completely remove this module/class by switching to explicit binding points in shaders, as every other graphics API work that way.

namespace Kakadu::RHI
{
	/* Forward declaration: */
	class Shader;
}

namespace Kakadu::RHI::Uniform
{
	using BindingPoint = u32;

	class BlockBindingPointBookKeeping
	{
	public:
		BlockBindingPointBookKeeping( const u32 start_offset, const u32 maximum_allowed )
			:
			start_offset( start_offset ),
			maximum_allowed( maximum_allowed )
		{}

		bool HasRoom() const
		{
			return ( in_use + 1 ) < maximum_allowed;
		}

		BindingPoint Assign( const std::string& block_name )
		{
			return binding_point_map[ block_name ] = start_offset + in_use++;
		}

		i32 InUseCount() const { return in_use; }

		bool IsAssigned( const std::string& block_name ) const
		{
			return binding_point_map.contains( block_name );
		}

		/*BindingPoint BindingPoint( const std::string& block_name ) const
		{
			return binding_point_map.at( block_name );
		}*/

		std::optional< const BindingPoint > Find( const std::string& block_name ) const
		{
			if( const auto iterator = binding_point_map.find( block_name );
				iterator != binding_point_map.cend() )
				return iterator->second;

			return std::nullopt;
		}

	private:
		std::unordered_map< std::string, BindingPoint > binding_point_map;
		u32 in_use = 0;
		u32 start_offset;
		u32 maximum_allowed;
		/* 4 bytes of padding. */;
	};

	// Singleton.
	class BlockBindingPointManager
	{
	public:
		DELETE_COPY_AND_MOVE_CONSTRUCTORS( BlockBindingPointManager );

	/* Block registering API: */
		static BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, BufferInformation& uniform_buffer_info );

	/* Buffer-Block connecting API: */
		static void ConnectBufferToBlock( const Buffer& uniform_buffer, const std::string& block_name, const BufferCategory category = BufferCategory::Regular );

	private:
	/* Private default constructor. */
		BlockBindingPointManager();

	/* Singleton related: */
		static BlockBindingPointManager& Instance()
		{
			local_persist BlockBindingPointManager instance;
			return instance;
		}

	/* Block registering API: */
		static BindingPoint RegisterUniformBlock( const Shader& shader, const std::string& block_name, BlockBindingPointBookKeeping& binding_point_book_keeping );

	/* Buffer-Block connecting API: */
		static void BindBufferToBindingPoint( const Buffer&, const BindingPoint );
		static void BindBufferToBindingPoint_Partial( const Buffer&, const BindingPoint,
													  const u32 offset, const u32 size );

	private:
		u32 binding_point_max_count;
		/* 4 bytes of padding. */

		BlockBindingPointBookKeeping binding_point_book_keeping_intrinsic;
		BlockBindingPointBookKeeping binding_point_book_keeping_global;
		BlockBindingPointBookKeeping binding_point_book_keeping_regular;
	};
}