// Engine Includes.
#include "RHI.h"
#include "DataType.h"
#include "Core/Assertion.h"
#include "Core/Optimization.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <stdexcept>
#include <unordered_map>

namespace Kakadu::RHI
{
	u32 DataTypeToGLEnum( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float   : return GL_FLOAT;
			case DataType::Float2  : return GL_FLOAT_VEC2;
			case DataType::Float3  : return GL_FLOAT_VEC3;
			case DataType::Float4  : return GL_FLOAT_VEC4;

			case DataType::Double  : return GL_DOUBLE;
			case DataType::Double2 : return GL_DOUBLE_VEC2;
			case DataType::Double3 : return GL_DOUBLE_VEC3;
			case DataType::Double4 : return GL_DOUBLE_VEC4;

			case DataType::Int     : return GL_INT;
			case DataType::Int2    : return GL_INT_VEC2;
			case DataType::Int3    : return GL_INT_VEC3;
			case DataType::Int4    : return GL_INT_VEC4;

			case DataType::UnsignedInt    : return GL_UNSIGNED_INT;
			case DataType::UnsignedInt2   : return GL_UNSIGNED_INT_VEC2;
			case DataType::UnsignedInt3   : return GL_UNSIGNED_INT_VEC3;
			case DataType::UnsignedInt4   : return GL_UNSIGNED_INT_VEC4;

			case DataType::Bool    : return GL_BOOL;
			case DataType::Bool2   : return GL_BOOL_VEC2;
			case DataType::Bool3   : return GL_BOOL_VEC3;
			case DataType::Bool4   : return GL_BOOL_VEC4;

			/* Float matrices: */
			case DataType::Float2x2 : return GL_FLOAT_MAT2;
			case DataType::Float3x3 : return GL_FLOAT_MAT3;
			case DataType::Float4x4 : return GL_FLOAT_MAT4;

			case DataType::Float2x3 : return GL_FLOAT_MAT2x3;
			case DataType::Float2x4 : return GL_FLOAT_MAT2x4;
			case DataType::Float3x2 : return GL_FLOAT_MAT3x2;
			case DataType::Float3x4 : return GL_FLOAT_MAT3x4;
			case DataType::Float4x2 : return GL_FLOAT_MAT4x2;
			case DataType::Float4x3 : return GL_FLOAT_MAT4x3;

			/* Double matrices: */
			case DataType::Double2x2 : return GL_DOUBLE_MAT2;
			case DataType::Double3x3 : return GL_DOUBLE_MAT3;
			case DataType::Double4x4 : return GL_DOUBLE_MAT4;

			case DataType::Double2x3 : return GL_DOUBLE_MAT2x3;
			case DataType::Double2x4 : return GL_DOUBLE_MAT2x4;
			case DataType::Double3x2 : return GL_DOUBLE_MAT3x2;
			case DataType::Double3x4 : return GL_DOUBLE_MAT3x4;
			case DataType::Double4x2 : return GL_DOUBLE_MAT4x2;
			case DataType::Double4x3 : return GL_DOUBLE_MAT4x3;

			/* Texture samplers: */
			case DataType::Sampler1D					: return GL_SAMPLER_1D;
			case DataType::Sampler2D					: return GL_SAMPLER_2D;
			case DataType::Sampler3D					: return GL_SAMPLER_3D;
			case DataType::SamplerCube					: return GL_SAMPLER_CUBE;
			case DataType::Sampler1DShadow				: return GL_SAMPLER_1D_SHADOW;
			case DataType::Sampler2DShadow				: return GL_SAMPLER_2D_SHADOW;
			case DataType::Sampler1DArray				: return GL_SAMPLER_1D_ARRAY;
			case DataType::Sampler2DArray				: return GL_SAMPLER_2D_ARRAY;
			case DataType::Sampler1DArrayShadow			: return GL_SAMPLER_1D_ARRAY_SHADOW;
			case DataType::Sampler2DArrayShadow			: return GL_SAMPLER_2D_ARRAY_SHADOW;
			case DataType::Sampler2DMS					: return GL_SAMPLER_2D_MULTISAMPLE;
			case DataType::Sampler2DMSArray				: return GL_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case DataType::SamplerCubeShadow			: return GL_SAMPLER_CUBE_SHADOW;
			case DataType::SamplerBuffer				: return GL_SAMPLER_BUFFER;
			case DataType::Sampler2DRect				: return GL_SAMPLER_2D_RECT;
			case DataType::Sampler2DRectShadow			: return GL_SAMPLER_2D_RECT_SHADOW;
			case DataType::IntSampler1D					: return GL_INT_SAMPLER_1D;
			case DataType::IntSampler2D					: return GL_INT_SAMPLER_2D;
			case DataType::IntSampler3D					: return GL_INT_SAMPLER_3D;
			case DataType::IntSamplerCube				: return GL_INT_SAMPLER_CUBE;
			case DataType::IntSampler1DArray			: return GL_INT_SAMPLER_1D_ARRAY;
			case DataType::IntSampler2DArray			: return GL_INT_SAMPLER_2D_ARRAY;
			case DataType::IntSampler2DMS				: return GL_INT_SAMPLER_2D_MULTISAMPLE;
			case DataType::IntSampler2DMSArray			: return GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case DataType::IntSamplerBuffer				: return GL_INT_SAMPLER_BUFFER;
			case DataType::IntSampler2DRect				: return GL_INT_SAMPLER_2D_RECT;
			case DataType::UnsignedIntSampler1D			: return GL_UNSIGNED_INT_SAMPLER_1D;
			case DataType::UnsignedIntSampler2D			: return GL_UNSIGNED_INT_SAMPLER_2D;
			case DataType::UnsignedIntSampler3D			: return GL_UNSIGNED_INT_SAMPLER_3D;
			case DataType::UnsignedIntSamplerCube		: return GL_UNSIGNED_INT_SAMPLER_CUBE;
			case DataType::UnsignedIntSampler1DArray	: return GL_UNSIGNED_INT_SAMPLER_1D_ARRAY;
			case DataType::UnsignedIntSampler2DArray	: return GL_UNSIGNED_INT_SAMPLER_2D_ARRAY;
			case DataType::UnsignedIntSampler2DMS		: return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE;
			case DataType::UnsignedIntSampler2DMSArray	: return GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY;
			case DataType::UnsignedIntSamplerBuffer		: return GL_UNSIGNED_INT_SAMPLER_BUFFER;
			case DataType::UnsignedIntSampler2DRect		: return GL_UNSIGNED_INT_SAMPLER_2D_RECT;
		}

		ASSERT( false && "Invalid DataType in Kakadu::RHI::DataTypeToGLEnum()!" );
		return GL_NONE;
	}

	DataType GLEnumToDataType( const u32 gl_enum )
	{
		switch( gl_enum )
		{
			/* Scalars & vectors: */
			case GL_FLOAT               : return DataType::Float;
			case GL_FLOAT_VEC2          : return DataType::Float2;
			case GL_FLOAT_VEC3          : return DataType::Float3;
			case GL_FLOAT_VEC4          : return DataType::Float4;

			case GL_DOUBLE              : return DataType::Double;
			case GL_DOUBLE_VEC2         : return DataType::Double2;
			case GL_DOUBLE_VEC3         : return DataType::Double3;
			case GL_DOUBLE_VEC4         : return DataType::Double4;

			case GL_INT                 : return DataType::Int;
			case GL_INT_VEC2            : return DataType::Int2;
			case GL_INT_VEC3            : return DataType::Int3;
			case GL_INT_VEC4            : return DataType::Int4;

			case GL_UNSIGNED_INT        : return DataType::UnsignedInt;
			case GL_UNSIGNED_INT_VEC2   : return DataType::UnsignedInt2;
			case GL_UNSIGNED_INT_VEC3   : return DataType::UnsignedInt3;
			case GL_UNSIGNED_INT_VEC4   : return DataType::UnsignedInt4;

			case GL_BOOL                : return DataType::Bool;
			case GL_BOOL_VEC2           : return DataType::Bool2;
			case GL_BOOL_VEC3           : return DataType::Bool3;
			case GL_BOOL_VEC4           : return DataType::Bool4;

			/* Float matrices: */
			case GL_FLOAT_MAT2          : return DataType::Float2x2;
			case GL_FLOAT_MAT3          : return DataType::Float3x3;
			case GL_FLOAT_MAT4          : return DataType::Float4x4;

			case GL_FLOAT_MAT2x3        : return DataType::Float2x3;
			case GL_FLOAT_MAT2x4        : return DataType::Float2x4;
			case GL_FLOAT_MAT3x2        : return DataType::Float3x2;
			case GL_FLOAT_MAT3x4        : return DataType::Float3x4;
			case GL_FLOAT_MAT4x2        : return DataType::Float4x2;
			case GL_FLOAT_MAT4x3        : return DataType::Float4x3;

			/* Double matrices: */
			case GL_DOUBLE_MAT2         : return DataType::Double2x2;
			case GL_DOUBLE_MAT3         : return DataType::Double3x3;
			case GL_DOUBLE_MAT4         : return DataType::Double4x4;

			case GL_DOUBLE_MAT2x3       : return DataType::Double2x3;
			case GL_DOUBLE_MAT2x4       : return DataType::Double2x4;
			case GL_DOUBLE_MAT3x2       : return DataType::Double3x2;
			case GL_DOUBLE_MAT3x4       : return DataType::Double3x4;
			case GL_DOUBLE_MAT4x2       : return DataType::Double4x2;
			case GL_DOUBLE_MAT4x3       : return DataType::Double4x3;

			/* Texture samplers: */
			case GL_SAMPLER_1D									: return DataType::Sampler1D;
			case GL_SAMPLER_2D									: return DataType::Sampler2D;
			case GL_SAMPLER_3D									: return DataType::Sampler3D;
			case GL_SAMPLER_CUBE								: return DataType::SamplerCube;
			case GL_SAMPLER_1D_SHADOW							: return DataType::Sampler1DShadow;
			case GL_SAMPLER_2D_SHADOW							: return DataType::Sampler2DShadow;
			case GL_SAMPLER_1D_ARRAY							: return DataType::Sampler1DArray;
			case GL_SAMPLER_2D_ARRAY							: return DataType::Sampler2DArray;
			case GL_SAMPLER_1D_ARRAY_SHADOW						: return DataType::Sampler1DArrayShadow;
			case GL_SAMPLER_2D_ARRAY_SHADOW						: return DataType::Sampler2DArrayShadow;
			case GL_SAMPLER_2D_MULTISAMPLE						: return DataType::Sampler2DMS;
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY				: return DataType::Sampler2DMSArray;
			case GL_SAMPLER_CUBE_SHADOW							: return DataType::SamplerCubeShadow;
			case GL_SAMPLER_BUFFER								: return DataType::SamplerBuffer;
			case GL_SAMPLER_2D_RECT								: return DataType::Sampler2DRect;
			case GL_SAMPLER_2D_RECT_SHADOW						: return DataType::Sampler2DRectShadow;
			case GL_INT_SAMPLER_1D								: return DataType::IntSampler1D;
			case GL_INT_SAMPLER_2D								: return DataType::IntSampler2D;
			case GL_INT_SAMPLER_3D								: return DataType::IntSampler3D;
			case GL_INT_SAMPLER_CUBE							: return DataType::IntSamplerCube;
			case GL_INT_SAMPLER_1D_ARRAY						: return DataType::IntSampler1DArray;
			case GL_INT_SAMPLER_2D_ARRAY						: return DataType::IntSampler2DArray;
			case GL_INT_SAMPLER_2D_MULTISAMPLE					: return DataType::IntSampler2DMS;
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY			: return DataType::IntSampler2DMSArray;
			case GL_INT_SAMPLER_BUFFER							: return DataType::IntSamplerBuffer;
			case GL_INT_SAMPLER_2D_RECT							: return DataType::IntSampler2DRect;
			case GL_UNSIGNED_INT_SAMPLER_1D						: return DataType::UnsignedIntSampler1D;
			case GL_UNSIGNED_INT_SAMPLER_2D						: return DataType::UnsignedIntSampler2D;
			case GL_UNSIGNED_INT_SAMPLER_3D						: return DataType::UnsignedIntSampler3D;
			case GL_UNSIGNED_INT_SAMPLER_CUBE					: return DataType::UnsignedIntSamplerCube;
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY				: return DataType::UnsignedIntSampler1DArray;
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY				: return DataType::UnsignedIntSampler2DArray;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE			: return DataType::UnsignedIntSampler2DMS;
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY	: return DataType::UnsignedIntSampler2DMSArray;
			case GL_UNSIGNED_INT_SAMPLER_BUFFER					: return DataType::UnsignedIntSamplerBuffer;
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT				: return DataType::UnsignedIntSampler2DRect;
		}

		ASSERT( false && "Invalid GL enum in Kakadu::RHI::GLEnumToDataType()!" );
		return DataType::Float;
	}

	i32 SizeOf( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float   : return sizeof( float );
			case DataType::Float2  : return sizeof( Vector2 );
			case DataType::Float3  : return sizeof( Vector3 );
			case DataType::Float4  : return sizeof( Vector4 );

			case DataType::Double  : return sizeof( double );
			case DataType::Double2 : return sizeof( Vector2D );
			case DataType::Double3 : return sizeof( Vector3D );
			case DataType::Double4 : return sizeof( Vector4D );

			case DataType::Int     : return sizeof( i32 );
			case DataType::Int2    : return sizeof( Vector2I );
			case DataType::Int3    : return sizeof( Vector3I );
			case DataType::Int4    : return sizeof( Vector4I );

			case DataType::UnsignedInt    : return sizeof( u32 );
			case DataType::UnsignedInt2   : return sizeof( Vector2U );
			case DataType::UnsignedInt3   : return sizeof( Vector3U );
			case DataType::UnsignedInt4   : return sizeof( Vector4U );

			case DataType::Bool    : return sizeof( bool );
			case DataType::Bool2   : return sizeof( bool ) * 2;
			case DataType::Bool3   : return sizeof( bool ) * 3;
			case DataType::Bool4   : return sizeof( bool ) * 4;

			/* Float matrices: */
			case DataType::Float2x2 : return sizeof( Matrix2x2 );
			case DataType::Float3x3 : return sizeof( Matrix3x3 );
			case DataType::Float4x4 : return sizeof( Matrix4x4 );

			case DataType::Float2x3 : return sizeof( Matrix2x3 );
			case DataType::Float2x4 : return sizeof( Matrix2x4 );
			case DataType::Float3x2 : return sizeof( Matrix3x2 );
			case DataType::Float3x4 : return sizeof( Matrix3x4 );
			case DataType::Float4x2 : return sizeof( Matrix4x2 );
			case DataType::Float4x3 : return sizeof( Matrix4x3 );

			/* Double matrices: */
			case DataType::Double2x2 : return sizeof( Matrix2x2D );
			case DataType::Double3x3 : return sizeof( Matrix3x3D );
			case DataType::Double4x4 : return sizeof( Matrix4x4D );

			case DataType::Double2x3 : return sizeof( Matrix2x3D );
			case DataType::Double2x4 : return sizeof( Matrix2x4D );
			case DataType::Double3x2 : return sizeof( Matrix3x2D );
			case DataType::Double3x4 : return sizeof( Matrix3x4D );
			case DataType::Double4x2 : return sizeof( Matrix4x2D );
			case DataType::Double4x3 : return sizeof( Matrix4x3D );

			/* Texture samplers: */
			case DataType::Sampler1D:
			case DataType::Sampler2D:
			case DataType::Sampler3D:
			case DataType::SamplerCube:
			case DataType::Sampler1DShadow:
			case DataType::Sampler2DShadow:
			case DataType::Sampler1DArray:
			case DataType::Sampler2DArray:
			case DataType::Sampler1DArrayShadow:
			case DataType::Sampler2DArrayShadow:
			case DataType::Sampler2DMS:
			case DataType::Sampler2DMSArray:
			case DataType::SamplerCubeShadow:
			case DataType::SamplerBuffer:
			case DataType::Sampler2DRect:
			case DataType::Sampler2DRectShadow:
			case DataType::IntSampler1D:
			case DataType::IntSampler2D:
			case DataType::IntSampler3D:
			case DataType::IntSamplerCube:
			case DataType::IntSampler1DArray:
			case DataType::IntSampler2DArray:
			case DataType::IntSampler2DMS:
			case DataType::IntSampler2DMSArray:
			case DataType::IntSamplerBuffer:
			case DataType::IntSampler2DRect:
			case DataType::UnsignedIntSampler1D:
			case DataType::UnsignedIntSampler2D:
			case DataType::UnsignedIntSampler3D:
			case DataType::UnsignedIntSamplerCube:
			case DataType::UnsignedIntSampler1DArray:
			case DataType::UnsignedIntSampler2DArray:
			case DataType::UnsignedIntSampler2DMS:
			case DataType::UnsignedIntSampler2DMSArray:
			case DataType::UnsignedIntSamplerBuffer:
			case DataType::UnsignedIntSampler2DRect:
				return sizeof( i32 );
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::SizeOf() called with an unknown DataType!" );
	}

	i32 CountOf( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float   : return 1;
			case DataType::Float2  : return 2;
			case DataType::Float3  : return 3;
			case DataType::Float4  : return 4;

			case DataType::Double  : return 1;
			case DataType::Double2 : return 2;
			case DataType::Double3 : return 3;
			case DataType::Double4 : return 4;

			case DataType::Int     : return 1;
			case DataType::Int2    : return 2;
			case DataType::Int3    : return 3;
			case DataType::Int4    : return 4;

			case DataType::UnsignedInt    : return 1;
			case DataType::UnsignedInt2   : return 2;
			case DataType::UnsignedInt3   : return 3;
			case DataType::UnsignedInt4   : return 4;

			case DataType::Bool    : return 1;
			case DataType::Bool2   : return 2;
			case DataType::Bool3   : return 3;
			case DataType::Bool4   : return 4;

			/* Float matrices: */
			case DataType::Float2x2 : return 4;
			case DataType::Float3x3 : return 9;
			case DataType::Float4x4 : return 16;

			case DataType::Float2x3 : return 6;
			case DataType::Float2x4 : return 8;
			case DataType::Float3x2 : return 6;
			case DataType::Float3x4 : return 12;
			case DataType::Float4x2 : return 8;
			case DataType::Float4x3 : return 12;

			/* Double matrices: */
			case DataType::Double2x2 : return 4;
			case DataType::Double3x3 : return 9;
			case DataType::Double4x4 : return 16;

			case DataType::Double2x3 : return 6;
			case DataType::Double2x4 : return 8;
			case DataType::Double3x2 : return 6;
			case DataType::Double3x4 : return 12;
			case DataType::Double4x2 : return 8;
			case DataType::Double4x3 : return 12;

			default:
				UNREACHABLE();
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::CountOf() called with an unknown DataType!" );
	}

	std::pair< i32, i32 > RowAndColumnCountOf( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float   : return { 1, 1 };
			case DataType::Float2  : return { 1, 2 };
			case DataType::Float3  : return { 1, 3 };
			case DataType::Float4  : return { 1, 4 };

			case DataType::Double  : return { 1, 1 };
			case DataType::Double2 : return { 1, 2 };
			case DataType::Double3 : return { 1, 3 };
			case DataType::Double4 : return { 1, 4 };

			case DataType::Int     : return { 1, 1 };
			case DataType::Int2    : return { 1, 2 };
			case DataType::Int3    : return { 1, 3 };
			case DataType::Int4    : return { 1, 4 };

			case DataType::UnsignedInt    : return { 1, 1 };
			case DataType::UnsignedInt2   : return { 1, 2 };
			case DataType::UnsignedInt3   : return { 1, 3 };
			case DataType::UnsignedInt4   : return { 1, 4 };

			case DataType::Bool    : return { 1, 1 };
			case DataType::Bool2   : return { 1, 2 };
			case DataType::Bool3   : return { 1, 3 };
			case DataType::Bool4   : return { 1, 4 };

			/* Float matrices: */
			case DataType::Float2x2 : return { 2, 2 };
			case DataType::Float3x3 : return { 3, 3 };
			case DataType::Float4x4 : return { 4, 4 };

			case DataType::Float2x3 : return { 2, 3 };
			case DataType::Float2x4 : return { 2, 4 };
			case DataType::Float3x2 : return { 3, 2 };
			case DataType::Float3x4 : return { 3, 4 };
			case DataType::Float4x2 : return { 4, 2 };
			case DataType::Float4x3 : return { 4, 3 };

			/* Double matrices: */
			case DataType::Double2x2 : return { 2, 2 };
			case DataType::Double3x3 : return { 3, 3 };
			case DataType::Double4x4 : return { 4, 4 };

			case DataType::Double2x3 : return { 2, 3 };
			case DataType::Double2x4 : return { 2, 4 };
			case DataType::Double3x2 : return { 3, 2 };
			case DataType::Double3x4 : return { 3, 4 };
			case DataType::Double4x2 : return { 4, 2 };
			case DataType::Double4x3 : return { 4, 3 };

			default:
				UNREACHABLE();
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::RowAndColumnCountOf() called with an unknown DataType!" );
	}

	const char* NameOf( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float   : return "float";
			case DataType::Float2  : return "vec2";
			case DataType::Float3  : return "vec3";
			case DataType::Float4  : return "vec4";

			case DataType::Double  : return "double";
			case DataType::Double2 : return "dvec2";
			case DataType::Double3 : return "dvec3";
			case DataType::Double4 : return "dvec4";

			case DataType::Int     : return "int";
			case DataType::Int2    : return "ivec2";
			case DataType::Int3    : return "ivec3";
			case DataType::Int4    : return "ivec4";

			case DataType::UnsignedInt    : return "uint";
			case DataType::UnsignedInt2   : return "uvec2";
			case DataType::UnsignedInt3   : return "uvec3";
			case DataType::UnsignedInt4   : return "uvec4";

			case DataType::Bool    : return "bool";
			case DataType::Bool2   : return "bvec2";
			case DataType::Bool3   : return "bvec3";
			case DataType::Bool4   : return "bvec4";

			/* Float matrices: */
			case DataType::Float2x2 : return "mat2";
			case DataType::Float3x3 : return "mat3";
			case DataType::Float4x4 : return "mat4";

			case DataType::Float2x3 : return "mat2x3";
			case DataType::Float2x4 : return "mat2x4";
			case DataType::Float3x2 : return "mat3x2";
			case DataType::Float3x4 : return "mat3x4";
			case DataType::Float4x2 : return "mat4x2";
			case DataType::Float4x3 : return "mat4x3";

			/* Double matrices: */
			case DataType::Double2x2 : return "dmat2";
			case DataType::Double3x3 : return "dmat3";
			case DataType::Double4x4 : return "dmat4";

			case DataType::Double2x3 : return "dmat2x3";
			case DataType::Double2x4 : return "dmat2x4";
			case DataType::Double3x2 : return "dmat3x2";
			case DataType::Double3x4 : return "dmat3x4";
			case DataType::Double4x2 : return "dmat4x2";
			case DataType::Double4x3 : return "dmat4x3";

			/* Texture samplers: */
			case DataType::Sampler1D					: return "sampler1D";
			case DataType::Sampler2D					: return "sampler2D";
			case DataType::Sampler3D					: return "sampler3D";
			case DataType::SamplerCube					: return "samplerCube";
			case DataType::Sampler1DShadow				: return "sampler1DShadow";
			case DataType::Sampler2DShadow				: return "sampler2DShadow";
			case DataType::Sampler1DArray				: return "sampler1DArray";
			case DataType::Sampler2DArray				: return "sampler2DArray";
			case DataType::Sampler1DArrayShadow			: return "sampler1DArrayShadow";
			case DataType::Sampler2DArrayShadow			: return "sampler2DArrayShadow";
			case DataType::Sampler2DMS					: return "sampler2DMS";
			case DataType::Sampler2DMSArray				: return "sampler2DMSArray";
			case DataType::SamplerCubeShadow			: return "samplerCubeShadow";
			case DataType::SamplerBuffer				: return "samplerBuffer";
			case DataType::Sampler2DRect				: return "sampler2DRect";
			case DataType::Sampler2DRectShadow			: return "sampler2DRectShadow";
			case DataType::IntSampler1D					: return "isampler1D";
			case DataType::IntSampler2D					: return "isampler2D";
			case DataType::IntSampler3D					: return "isampler3D";
			case DataType::IntSamplerCube				: return "isamplerCube";
			case DataType::IntSampler1DArray			: return "isampler1DArray";
			case DataType::IntSampler2DArray			: return "isampler2DArray";
			case DataType::IntSampler2DMS				: return "isampler2DMS";
			case DataType::IntSampler2DMSArray			: return "isampler2DMSArray";
			case DataType::IntSamplerBuffer				: return "isamplerBuffer";
			case DataType::IntSampler2DRect				: return "isampler2DRect";
			case DataType::UnsignedIntSampler1D         : return "usampler1D";
			case DataType::UnsignedIntSampler2D         : return "usampler2D";
			case DataType::UnsignedIntSampler3D         : return "usampler3D";
			case DataType::UnsignedIntSamplerCube       : return "usamplerCube";
			case DataType::UnsignedIntSampler1DArray    : return "usampler1DArray";
			case DataType::UnsignedIntSampler2DArray    : return "usampler2DArray";
			case DataType::UnsignedIntSampler2DMS       : return "usampler2DMS";
			case DataType::UnsignedIntSampler2DMSArray  : return "usampler2DMSArray";
			case DataType::UnsignedIntSamplerBuffer     : return "usamplerBuffer";
			case DataType::UnsignedIntSampler2DRect     : return "usampler2DRect";
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::NameOf() called with an unknown DataType!" );
	}

	DataType TypeOf( const char* name )
	{
		local_persist std::unordered_map< std::string, DataType > lookup_table =
		{
			/* Scalars & vectors: */
			{ "float",  DataType::Float  },
			{ "vec2",   DataType::Float2 },
			{ "vec3",   DataType::Float3 },
			{ "vec4",   DataType::Float4 },

			{ "double", DataType::Double  },
			{ "dvec2",  DataType::Double2 },
			{ "dvec3",  DataType::Double3 },
			{ "dvec4",  DataType::Double4 },

			{ "int",    DataType::Int  },
			{ "ivec2",  DataType::Int2 },
			{ "ivec3",  DataType::Int3 },
			{ "ivec4",  DataType::Int4 },

			{ "uint",   DataType::UnsignedInt  },
			{ "uvec2",  DataType::UnsignedInt2 },
			{ "uvec3",  DataType::UnsignedInt3 },
			{ "uvec4",  DataType::UnsignedInt4 },

			{ "bool",   DataType::Bool  },
			{ "bvec2",  DataType::Bool2 },
			{ "bvec3",  DataType::Bool3 },
			{ "bvec4",  DataType::Bool4 },

			/* Float matrices: */
			{ "mat2",   DataType::Float2x2 },
			{ "mat3",   DataType::Float3x3 },
			{ "mat4",   DataType::Float4x4 },

			{ "mat2x3", DataType::Float2x3 },
			{ "mat2x4", DataType::Float2x4 },
			{ "mat3x2", DataType::Float3x2 },
			{ "mat3x4", DataType::Float3x4 },
			{ "mat4x2", DataType::Float4x2 },
			{ "mat4x3", DataType::Float4x3 },

			/* Double matrices: */
			{ "dmat2",   DataType::Double2x2 },
			{ "dmat3",   DataType::Double3x3 },
			{ "dmat4",   DataType::Double4x4 },

			{ "dmat2x3", DataType::Double2x3 },
			{ "dmat2x4", DataType::Double2x4 },
			{ "dmat3x2", DataType::Double3x2 },
			{ "dmat3x4", DataType::Double3x4 },
			{ "dmat4x2", DataType::Double4x2 },
			{ "dmat4x3", DataType::Double4x3 },

			/* Texture samplers: */
			{ "sampler1D",              DataType::Sampler1D						},
			{ "sampler2D",              DataType::Sampler2D						},
			{ "sampler3D",              DataType::Sampler3D						},
			{ "samplerCube",            DataType::SamplerCube					},
			{ "sampler1DShadow",        DataType::Sampler1DShadow				},
			{ "sampler2DShadow",        DataType::Sampler2DShadow				},
			{ "sampler1DArray",         DataType::Sampler1DArray				},
			{ "sampler2DArray",         DataType::Sampler2DArray				},
			{ "sampler1DArrayShadow",   DataType::Sampler1DArrayShadow			},
			{ "sampler2DArrayShadow",   DataType::Sampler2DArrayShadow			},
			{ "sampler2DMS",            DataType::Sampler2DMS					},
			{ "sampler2DMSArray",       DataType::Sampler2DMSArray				},
			{ "samplerCubeShadow",      DataType::SamplerCubeShadow				},
			{ "samplerBuffer",          DataType::SamplerBuffer					},
			{ "sampler2DRect",          DataType::Sampler2DRect					},
			{ "sampler2DRectShadow",    DataType::Sampler2DRectShadow			},
			{ "isampler1D",             DataType::IntSampler1D					},
			{ "isampler2D",             DataType::IntSampler2D					},
			{ "isampler3D",             DataType::IntSampler3D					},
			{ "isamplerCube",           DataType::IntSamplerCube				},
			{ "isampler1DArray",        DataType::IntSampler1DArray				},
			{ "isampler2DArray",        DataType::IntSampler2DArray				},
			{ "isampler2DMS",           DataType::IntSampler2DMS				},
			{ "isampler2DMSArray",      DataType::IntSampler2DMSArray			},
			{ "isamplerBuffer",         DataType::IntSamplerBuffer				},
			{ "isampler2DRect",         DataType::IntSampler2DRect				},
			{ "usampler1D",             DataType::UnsignedIntSampler1D			},
			{ "usampler2D",             DataType::UnsignedIntSampler2D			},
			{ "usampler3D",             DataType::UnsignedIntSampler3D			},
			{ "usamplerCube",           DataType::UnsignedIntSamplerCube		},
			{ "usampler1DArray",        DataType::UnsignedIntSampler1DArray		},
			{ "usampler2DArray",        DataType::UnsignedIntSampler2DArray		},
			{ "usampler2DMS",           DataType::UnsignedIntSampler2DMS		},
			{ "usampler2DMSArray",      DataType::UnsignedIntSampler2DMSArray	},
			{ "usamplerBuffer",         DataType::UnsignedIntSamplerBuffer		},
			{ "usampler2DRect",         DataType::UnsignedIntSampler2DRect		},
		};

		return lookup_table[ name ];
	}

	DataType ComponentTypeOf( const DataType type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case DataType::Float  :
			case DataType::Float2 :
			case DataType::Float3 :
			case DataType::Float4 : return DataType::Float;

			case DataType::Double  :
			case DataType::Double2 :
			case DataType::Double3 :
			case DataType::Double4 : return DataType::Double;

			case DataType::Int  :
			case DataType::Int2 :
			case DataType::Int3 :
			case DataType::Int4 : return DataType::Int;

			case DataType::UnsignedInt  :
			case DataType::UnsignedInt2 :
			case DataType::UnsignedInt3 :
			case DataType::UnsignedInt4 : return DataType::UnsignedInt;

			case DataType::Bool  :
			case DataType::Bool2 :
			case DataType::Bool3 :
			case DataType::Bool4 : return DataType::Bool;

			/* Float matrices: */
			case DataType::Float2x2 :
			case DataType::Float3x3 :
			case DataType::Float4x4 :
			case DataType::Float2x3 :
			case DataType::Float2x4 :
			case DataType::Float3x2 :
			case DataType::Float3x4 :
			case DataType::Float4x2 :
			case DataType::Float4x3 : return DataType::Float;

			/* Double matrices: */
			case DataType::Double2x2 :
			case DataType::Double3x3 :
			case DataType::Double4x4 :
			case DataType::Double2x3 :
			case DataType::Double2x4 :
			case DataType::Double3x2 :
			case DataType::Double3x4 :
			case DataType::Double4x2 :
			case DataType::Double4x3 : return DataType::Double;

			default:
				UNREACHABLE();
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::ComponentTypeOf() called with an unknown DataType!" );
	}

	void* AddressOf( const DataType type, void* address, const i32 element_index )
	{
		return ( void* )( ( uintptr_t )address + element_index * SizeOf( type ) );
	}

	const void* AddressOf( const DataType type, const void* address, const i32 element_index )
	{
		return ( const void* )( ( uintptr_t )address + element_index * SizeOf( type ) );
	}
}
