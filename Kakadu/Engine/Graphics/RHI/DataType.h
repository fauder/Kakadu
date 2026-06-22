#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <utility>

namespace Kakadu::RHI
{
	enum struct DataType : u32
	{
		/* Scalars & vectors: */
		Float,
		Float2,
		Float3,
		Float4,

		Double,
		Double2,
		Double3,
		Double4,

		Int,
		Int2,
		Int3,
		Int4,

		UnsignedInt,
		UnsignedInt2,
		UnsignedInt3,
		UnsignedInt4,

		Bool,
		Bool2,
		Bool3,
		Bool4,

		/* Float matrices: */
		Float2x2,
		Float3x3,
		Float4x4,

		Float2x3,
		Float2x4,
		Float3x2,
		Float3x4,
		Float4x2,
		Float4x3,

		/* Double matrices: */
		Double2x2,
		Double3x3,
		Double4x4,

		Double2x3,
		Double2x4,
		Double3x2,
		Double3x4,
		Double4x2,
		Double4x3,

		/* Texture samplers: */
		Sampler1D,
		Sampler2D,
		Sampler3D,

		SamplerCube,

		Sampler1DShadow,
		Sampler2DShadow,

		Sampler1DArray,
		Sampler2DArray,

		Sampler1DArrayShadow,
		Sampler2DArrayShadow,

		Sampler2DMS,
		Sampler2DMSArray,

		SamplerCubeShadow,

		SamplerBuffer,

		Sampler2DRect,
		Sampler2DRectShadow,

		IntSampler1D,
		IntSampler2D,
		IntSampler3D,

		IntSamplerCube,

		IntSampler1DArray,
		IntSampler2DArray,

		IntSampler2DMS,
		IntSampler2DMSArray,

		IntSamplerBuffer,
		IntSampler2DRect,

		UnsignedIntSampler1D,
		UnsignedIntSampler2D,
		UnsignedIntSampler3D,

		UnsignedIntSamplerCube,

		UnsignedIntSampler1DArray,
		UnsignedIntSampler2DArray,

		UnsignedIntSampler2DMS,
		UnsignedIntSampler2DMSArray,

		UnsignedIntSamplerBuffer,
		UnsignedIntSampler2DRect,
	};

	u32			DataTypeToGLEnum( DataType type );
	DataType	GLEnumToDataType( u32 gl_enum );

	i32						SizeOf( DataType type );
	i32						CountOf( DataType type );
	std::pair< i32, i32 >	RowAndColumnCountOf( DataType type );
	const char*				NameOf( DataType type );
	DataType				TypeOf( const char* name );
	DataType				ComponentTypeOf( DataType type );

	void*       AddressOf( DataType type, void* address, i32 element_index );
	const void* AddressOf( DataType type, const void* address, i32 element_index );
}
