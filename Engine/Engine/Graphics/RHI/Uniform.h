#pragma once

// Engine Includes.
#include "RHI.h"
#include "Graphics/UniformAnnotation.h" // TODO: Dependency - wrong direction. Fix later by moving this out of both Uniform::Information and Shader and keep elsewhere.
#include "Core/Blob.hpp"
#include "Core/Types.h"

// std Includes.
#include <array>
#include <optional>
#include <string>
#include <unordered_map>

namespace Kakadu::RHI::Uniform
{
	struct Information
	{
		i32 location_or_block_index; // Changes meaning depending on context; location if this is a stand-alone uniform, or the index of the block if this resides in a uniform buffer block.
		i32 size;
		i32 offset;
		i32 count_array; // Element count (>= 1) for arrays, 1 for non-arrays.

		GLenum type;
		bool is_buffer_member;

		UniformAnnotation::Type annotation_type;
		u16 annotation_format_string_id;
		std::array< u8, 16 > annotation_meta_data;

		std::string editor_name;
	};

	enum class BufferCategory
	{
		Regular,	// Per-material data; Same for every shader of a given material; differs per-material.
		Global,		// Global user-defined data. Same for every shader.
		Intrinsic	// Data known & set by the Renderer such as camera details; view & projection matrices. Same for every shader.
	};

	static BufferCategory DetermineBufferCategory( std::string_view buffer_name )
	{
		if( buffer_name.find( "_Global" ) != std::string_view::npos )
			return BufferCategory::Global;
		else if( buffer_name.find( "_Intrinsic" ) != std::string_view::npos )
			return BufferCategory::Intrinsic;
		else
			return BufferCategory::Regular;
	}

	struct BufferMemberInformation_Struct
	{
		i32 offset;
		i32 size;

		std::string editor_name;

		std::vector< Uniform::Information* > members_map;
	};

	struct BufferMemberInformation_Array
	{
		i32 offset;
		i32 stride;
		i32 element_count;
		/* 4 bytes of padding. */

		std::string editor_name;

		std::vector< Uniform::Information* > members_map;
	};

	struct BufferInformation
	{
	public:
		i32 binding_point;
		i32 size;
		i32 offset;

		BufferCategory category;

		std::unordered_map< std::string, Information*						> members_map;			// Key is qualified by the buffer name. Example: "Lighting.spot_light_data".
		std::unordered_map< std::string, BufferMemberInformation_Struct		> members_struct_map;	// Key is the uniform name alone. Example: "spot_light_data".
		std::unordered_map< std::string, BufferMemberInformation_Array		> members_array_map;	// Key is the uniform name alone. Example: "point_lights".
		std::unordered_map< std::string, Information*						> members_single_map;	// Key is the uniform name alone. Example: "color_modulation".

		bool IsRegular()	const { return category == BufferCategory::Regular;		}
		bool IsGlobal()		const { return category == BufferCategory::Global;		}
		bool IsIntrinsic()	const { return category == BufferCategory::Intrinsic;	}

		bool IsGlobalOrIntrinsic() const { return IsGlobal() || IsIntrinsic(); }
			
		static const char* CategoryString( const BufferCategory category )
		{
			switch( category )
			{
				case BufferCategory::Intrinsic:			return "Intrinsic";
				case BufferCategory::Global:			return "Global";
				default: /* BufferCategory::Regular */	return "Regular";
			}
		}
	};

	struct ActiveUniformBookKeepingInformation
	{
		std::string name_holder;
		i32 count;
		i32 name_max_length;
		std::size_t default_block_size;
		std::size_t regular_total_size, global_total_size, intrinsic_total_size;
		std::size_t total_size;
		i32 intrinsic_block_count, global_block_count, regular_block_count;
		/* 4 bytes of padding. */

		std::size_t TotalSize_ForMaterialBlob() const { return default_block_size + regular_total_size; }
		std::size_t TotalSize_Blocks()			const { return regular_total_size + global_total_size + intrinsic_total_size; }
	};
};
