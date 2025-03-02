#pragma once

// Engine Includes.
#include "Buffer.hpp"
#include "Std140StructTag.h"
#include "Uniform.h"
#include "UniformBufferManager.h"
#include "Core/DirtyBlob.h"

// std Includes.
#include <unordered_map>

namespace Engine
{
	template< typename BlobType > requires( std::is_base_of_v< Blob, BlobType > )
	class UniformBufferManagement
	{
	public:
		UniformBufferManagement() = default;

		DELETE_COPY_CONSTRUCTORS( UniformBufferManagement );

		DEFAULT_MOVE_CONSTRUCTORS( UniformBufferManagement );

		~UniformBufferManagement() = default;

	/* Queries: */
		inline const std::unordered_map< std::string, const Uniform::BufferInformation >& GetBufferInformationMap() const { return buffer_info_map; }

	/* Register/Unregister Buffer API: */
		void RegisterBuffer( const std::string& buffer_name, const Uniform::BufferInformation buffer_info )
		{
			if( buffer_info_map.try_emplace( buffer_name, buffer_info ).second ) // .second returns whether the emplace was successfull or not.
			{
				buffer_map.emplace( buffer_name, UniformBufferManager::CreateOrRequest( buffer_name, buffer_info ) );

				blob_map.emplace( buffer_name, buffer_info.size );
			}
		}

		/* This is used by the Renderer, for Intrinsics/Globals:
		 * Since Uniform::BufferInformation holds pointers to Shader data, it is effectively lost/corrupted when the host Shader is destroyed (upon recompilation). */
		void RegisterBuffer_ForceUpdateBufferInfoIfBufferExists( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info )
		{
			if( buffer_info_map.contains( buffer_name ) )
			{
				buffer_info_map.erase( buffer_name );
				buffer_info_map.emplace( buffer_name, buffer_info );
			}
			else
				RegisterBuffer( buffer_name, buffer_info );
		}

		void UnregisterBuffer( const std::string& buffer_name )
		{
			if( const auto& iterator = buffer_info_map.find( buffer_name );
				iterator != buffer_info_map.cend() )
			{
				buffer_map.erase( buffer_name );
				buffer_info_map.erase( buffer_name );
				blob_map.erase( buffer_name );
			}
		}

		void UnregisterAllBuffers()
		{
			if( not buffer_info_map.empty() )
			{
				buffer_map.clear();
				buffer_info_map.clear();
				blob_map.clear();
			}
		}

	/* Uniform Set/Get: */
		const void* Get( const std::string& buffer_name ) const
		{
			return blob_map.at( buffer_name ).Get( 0 );
		}

		void* Get( const std::string& buffer_name )
		{
			return blob_map[ buffer_name ].Get( 0 );
		}

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const std::string& buffer_name, const StructType& value )
		{
			const auto& buffer_info = buffer_info_map[ buffer_name ];

			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), 0 /* because every buffer has its own blob. */, buffer_info.size );
		}

		/* For PARTIAL setting of ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetPartial_Array( const std::string& buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			const auto& buffer_info              = buffer_info_map[ buffer_name ];
			const auto& buffer_member_array_info = buffer_info.members_array_map.at( uniform_member_array_instance_name );

			const auto effective_offset = buffer_member_array_info.offset + 
										  array_index * buffer_member_array_info.stride;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, buffer_member_array_info.stride );
		}

		/* For PARTIAL setting of ARRAY uniforms INSIDE a Uniform Buffer. */
		void SetPartial_Array( const std::string& buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const std::byte* value )
		{
			const auto& buffer_info              = buffer_info_map[ buffer_name ];
			const auto& buffer_member_array_info = buffer_info.members_array_map.at( uniform_member_array_instance_name );

			const auto effective_offset = buffer_member_array_info.offset + 
										  array_index * buffer_member_array_info.stride;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( value, effective_offset, buffer_member_array_info.stride );
		}

		/* For PARTIAL setting of STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetPartial_Struct( const std::string& buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			const auto& buffer_info               = buffer_info_map[ buffer_name ];
			const auto& buffer_member_struct_info = buffer_info.members_struct_map.at( uniform_member_struct_instance_name );

			const auto effective_offset = buffer_member_struct_info.offset;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), effective_offset, buffer_member_struct_info.size );
		}

		/* For PARTIAL setting of STRUCT uniforms INSIDE a Uniform Buffer. */
		void SetPartial_Struct( const std::string& buffer_name, const char* uniform_member_struct_instance_name, const std::byte* value )
		{
			const auto& buffer_info               = buffer_info_map[ buffer_name ];
			const auto& buffer_member_struct_info = buffer_info.members_struct_map.at( uniform_member_struct_instance_name );

			const auto effective_offset = buffer_member_struct_info.offset;

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( value, effective_offset, buffer_member_struct_info.size );
		}

		/* For PARTIAL setting of NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType > requires( not std::is_pointer_v< UniformType > ) // Don't want to "override" the overload below for actual pointer types.
		void SetPartial( const std::string& buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			const auto& buffer_info               = buffer_info_map[ buffer_name ];
			const auto& buffer_member_single_info = buffer_info.members_single_map.at( uniform_member_name );

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( reinterpret_cast< const std::byte* >( &value ), buffer_member_single_info->offset, buffer_member_single_info->size );
		}

		/* For PARTIAL setting of NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		void SetPartial( const std::string& buffer_name, const char* uniform_member_name, const std::byte* value )
		{
			const auto& buffer_info               = buffer_info_map[ buffer_name ];
			const auto& buffer_member_single_info = buffer_info.members_single_map.at( uniform_member_name );

			/* Update the value in the internal memory blob: */
			blob_map[ buffer_name ].Set( value, buffer_member_single_info->offset, buffer_member_single_info->size );
		}

	/* Uniform Upload: */
		void UploadAll()
		{
			for( auto& [ uniform_buffer_name, uniform_blob ] : blob_map )
			{
				const auto& uniform_buffer = buffer_map[ uniform_buffer_name ];

				if constexpr( std::is_same_v< BlobType, DirtyBlob > )
				{
					if( uniform_blob.IsDirty() )
					{
						uniform_blob.MergeConsecutiveDirtySections();
						const auto& dirty_sections = uniform_blob.DirtySections();
						for( auto& dirty_section : dirty_sections )
							uniform_buffer->Upload_Partial( uniform_blob.SpanFromSection( dirty_section ), dirty_section.offset );

						uniform_blob.ClearDirtySections();
					}
				}
				else // Regular Blob.
				{
					uniform_buffer->Upload( uniform_blob.Get( 0 ) );
				}
			}
		}

	private:
		// TODO: Use hashes instead of strings as keys.

		std::unordered_map< std::string, const Uniform::BufferInformation > buffer_info_map;

		std::unordered_map< std::string, UniformBuffer* > buffer_map;

		std::unordered_map< std::string, BlobType > blob_map;
	};
}
