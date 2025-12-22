#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <stdexcept>
#include <unordered_map>
#include <variant>

namespace Engine::GL::Type
{
	using ShaderTypesVariant = 
		std::variant<
			/* Scalars & vectors: */
			float,			Vector2,	Vector3,	Vector4,
			double,			Vector2D,	Vector3D,	Vector4D,
			int,			Vector2I,	Vector3I,	Vector4I,
			unsigned int,	Vector2U,	Vector3U,	Vector4U,
			bool,			Vector2B,	Vector3B,	Vector4B,
		
			/* Float matrices: */
							Matrix2x2,	Matrix3x3,	Matrix4x4,
							Matrix2x3,	Matrix2x4,
							Matrix3x2,	Matrix3x4,
							Matrix4x2,	Matrix4x3,

			/* Double matrices: */
							Matrix2x2D,	Matrix3x3D,	Matrix4x4D,
							Matrix2x3D,	Matrix2x4D,
							Matrix3x2D,	Matrix3x4D,
							Matrix4x2D,	Matrix4x3D >;

	header_function	int SizeOf( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: return sizeof( float );
			case GL_FLOAT_VEC2			: return sizeof( Vector2 );
			case GL_FLOAT_VEC3			: return sizeof( Vector3 );
			case GL_FLOAT_VEC4			: return sizeof( Vector4 );

			case GL_DOUBLE				: return sizeof( double );
			case GL_DOUBLE_VEC2			: return sizeof( Vector2D );
			case GL_DOUBLE_VEC3			: return sizeof( Vector3D );
			case GL_DOUBLE_VEC4			: return sizeof( Vector4D );

			case GL_INT					: return sizeof( int );
			case GL_INT_VEC2			: return sizeof( Vector2I );
			case GL_INT_VEC3			: return sizeof( Vector3I );
			case GL_INT_VEC4			: return sizeof( Vector4I );

			case GL_UNSIGNED_INT		: return sizeof( unsigned int );
			case GL_UNSIGNED_INT_VEC2	: return sizeof( Vector2U );
			case GL_UNSIGNED_INT_VEC3	: return sizeof( Vector3U );
			case GL_UNSIGNED_INT_VEC4	: return sizeof( Vector4U );

			case GL_BOOL				: return sizeof( bool );
			case GL_BOOL_VEC2			: return sizeof( Vector2B );
			case GL_BOOL_VEC3			: return sizeof( Vector3B );
			case GL_BOOL_VEC4			: return sizeof( Vector4B );

			/* Float matrices: */
			case GL_FLOAT_MAT2 			: return sizeof( Matrix2x2 );
			case GL_FLOAT_MAT3 			: return sizeof( Matrix3x3 );
			case GL_FLOAT_MAT4 			: return sizeof( Matrix4x4 );

			case GL_FLOAT_MAT2x3 		: return sizeof( Matrix2x3 );
			case GL_FLOAT_MAT2x4 		: return sizeof( Matrix2x4 );
			case GL_FLOAT_MAT3x2 		: return sizeof( Matrix3x2 );
			case GL_FLOAT_MAT3x4 		: return sizeof( Matrix3x4 );
			case GL_FLOAT_MAT4x2 		: return sizeof( Matrix4x2 );
			case GL_FLOAT_MAT4x3 		: return sizeof( Matrix4x3 );

			/* Double matrices: */
			case GL_DOUBLE_MAT2 		: return sizeof( Matrix2x2D );
			case GL_DOUBLE_MAT3 		: return sizeof( Matrix3x3D );
			case GL_DOUBLE_MAT4 		: return sizeof( Matrix4x4D );

			case GL_DOUBLE_MAT2x3 		: return sizeof( Matrix2x3D );
			case GL_DOUBLE_MAT2x4 		: return sizeof( Matrix2x4D );
			case GL_DOUBLE_MAT3x2 		: return sizeof( Matrix3x2D );
			case GL_DOUBLE_MAT3x4 		: return sizeof( Matrix3x4D );
			case GL_DOUBLE_MAT4x2 		: return sizeof( Matrix4x2D );
			case GL_DOUBLE_MAT4x3 		: return sizeof( Matrix4x3D );

			/* Texture samplers: */
			case GL_SAMPLER_1D:
			case GL_SAMPLER_2D:
			case GL_SAMPLER_3D:
			case GL_SAMPLER_CUBE:
			case GL_SAMPLER_1D_SHADOW:
			case GL_SAMPLER_2D_SHADOW:
			case GL_SAMPLER_1D_ARRAY:
			case GL_SAMPLER_2D_ARRAY:
			case GL_SAMPLER_1D_ARRAY_SHADOW:
			case GL_SAMPLER_2D_ARRAY_SHADOW:
			case GL_SAMPLER_2D_MULTISAMPLE:
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_SAMPLER_CUBE_SHADOW:
			case GL_SAMPLER_BUFFER:
			case GL_SAMPLER_2D_RECT:
			case GL_SAMPLER_2D_RECT_SHADOW:
			case GL_INT_SAMPLER_1D:
			case GL_INT_SAMPLER_2D:
			case GL_INT_SAMPLER_3D:
			case GL_INT_SAMPLER_CUBE:
			case GL_INT_SAMPLER_1D_ARRAY:
			case GL_INT_SAMPLER_2D_ARRAY:
			case GL_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_INT_SAMPLER_BUFFER:
			case GL_INT_SAMPLER_2D_RECT:
			case GL_UNSIGNED_INT_SAMPLER_1D:
			case GL_UNSIGNED_INT_SAMPLER_2D:
			case GL_UNSIGNED_INT_SAMPLER_3D:
			case GL_UNSIGNED_INT_SAMPLER_CUBE:
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
			case GL_UNSIGNED_INT_SAMPLER_BUFFER:
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
				return sizeof( int );
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::SizeOf() called with an unknown GL type!" );
	}

	header_function int CountOf( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: return 1;
			case GL_FLOAT_VEC2			: return 2;
			case GL_FLOAT_VEC3			: return 3;
			case GL_FLOAT_VEC4			: return 4;

			case GL_DOUBLE				: return 1;
			case GL_DOUBLE_VEC2			: return 2;
			case GL_DOUBLE_VEC3			: return 3;
			case GL_DOUBLE_VEC4			: return 4;

			case GL_INT					: return 1;
			case GL_INT_VEC2			: return 2;
			case GL_INT_VEC3			: return 3;
			case GL_INT_VEC4			: return 4;

			case GL_UNSIGNED_INT		: return 1;
			case GL_UNSIGNED_INT_VEC2	: return 2;
			case GL_UNSIGNED_INT_VEC3	: return 3;
			case GL_UNSIGNED_INT_VEC4	: return 4;

			case GL_BOOL				: return 1;
			case GL_BOOL_VEC2			: return 2;
			case GL_BOOL_VEC3			: return 3;
			case GL_BOOL_VEC4			: return 4;

			/* Float matrices: */
			case GL_FLOAT_MAT2 			: return 4;
			case GL_FLOAT_MAT3 			: return 9;
			case GL_FLOAT_MAT4 			: return 16;

			case GL_FLOAT_MAT2x3 		: return 6;
			case GL_FLOAT_MAT2x4 		: return 8;
			case GL_FLOAT_MAT3x2 		: return 6;
			case GL_FLOAT_MAT3x4 		: return 12;
			case GL_FLOAT_MAT4x2 		: return 8;
			case GL_FLOAT_MAT4x3 		: return 12;

			/* Double matrices: */
			case GL_DOUBLE_MAT2 		: return 4;
			case GL_DOUBLE_MAT3 		: return 9;
			case GL_DOUBLE_MAT4 		: return 16;

			case GL_DOUBLE_MAT2x3 		: return 6;
			case GL_DOUBLE_MAT2x4 		: return 8;
			case GL_DOUBLE_MAT3x2 		: return 6;
			case GL_DOUBLE_MAT3x4 		: return 12;
			case GL_DOUBLE_MAT4x2 		: return 8;
			case GL_DOUBLE_MAT4x3 		: return 12;
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::CountOf() called with an unknown GL type!" );
	}

	header_function	std::pair< int, int > RowAndColumnCountOf( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				: return  { 1, 1 };
			case GL_FLOAT_VEC2			: return  { 1, 2 };
			case GL_FLOAT_VEC3			: return  { 1, 3 };
			case GL_FLOAT_VEC4			: return  { 1, 4 };

			case GL_DOUBLE				: return  { 1, 1 };
			case GL_DOUBLE_VEC2			: return  { 1, 2 };
			case GL_DOUBLE_VEC3			: return  { 1, 3 };
			case GL_DOUBLE_VEC4			: return  { 1, 4 };

			case GL_INT					: return  { 1, 1 };
			case GL_INT_VEC2			: return  { 1, 2 };
			case GL_INT_VEC3			: return  { 1, 3 };
			case GL_INT_VEC4			: return  { 1, 4 };

			case GL_UNSIGNED_INT		: return  { 1, 1 };
			case GL_UNSIGNED_INT_VEC2	: return  { 1, 2 };
			case GL_UNSIGNED_INT_VEC3	: return  { 1, 3 };
			case GL_UNSIGNED_INT_VEC4	: return  { 1, 4 };

			case GL_BOOL				: return  { 1, 1 };
			case GL_BOOL_VEC2			: return  { 1, 2 };
			case GL_BOOL_VEC3			: return  { 1, 3 };
			case GL_BOOL_VEC4			: return  { 1, 4 };

			/* Float matrices: */
			case GL_FLOAT_MAT2 			: return  { 2, 2 };
			case GL_FLOAT_MAT3 			: return  { 3, 3 };
			case GL_FLOAT_MAT4 			: return  { 4, 4 };

			case GL_FLOAT_MAT2x3 		: return  { 2, 3 };
			case GL_FLOAT_MAT2x4 		: return  { 2, 4 };
			case GL_FLOAT_MAT3x2 		: return  { 3, 2 };
			case GL_FLOAT_MAT3x4 		: return  { 3, 4 };
			case GL_FLOAT_MAT4x2 		: return  { 4, 2 };
			case GL_FLOAT_MAT4x3 		: return  { 4, 3 };

			/* Double matrices: */
			case GL_DOUBLE_MAT2 		: return  { 2, 2 };
			case GL_DOUBLE_MAT3 		: return  { 3, 3 };
			case GL_DOUBLE_MAT4 		: return  { 4, 4 };

			case GL_DOUBLE_MAT2x3 		: return  { 2, 3 };
			case GL_DOUBLE_MAT2x4 		: return  { 2, 4 };
			case GL_DOUBLE_MAT3x2 		: return  { 3, 2 };
			case GL_DOUBLE_MAT3x4 		: return  { 3, 4 };
			case GL_DOUBLE_MAT4x2 		: return  { 4, 2 };
			case GL_DOUBLE_MAT4x3 		: return  { 4, 3 };
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::RowAndColumnCountOf() called with an unknown GL type!" );
	}

	header_function const char* NameOf( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT											: return "float";
			case GL_FLOAT_VEC2										: return "vec2";
			case GL_FLOAT_VEC3										: return "vec3";
			case GL_FLOAT_VEC4										: return "vec4";
			
			case GL_DOUBLE											: return "double";
			case GL_DOUBLE_VEC2										: return "dvec2";
			case GL_DOUBLE_VEC3										: return "dvec3";
			case GL_DOUBLE_VEC4										: return "dvec4";

			case GL_INT												: return "int";
			case GL_INT_VEC2										: return "ivec2";
			case GL_INT_VEC3										: return "ivec3";
			case GL_INT_VEC4										: return "ivec4";
			
			case GL_UNSIGNED_INT									: return "unsigned int";
			case GL_UNSIGNED_INT_VEC2								: return "uvec2";
			case GL_UNSIGNED_INT_VEC3								: return "uvec3";
			case GL_UNSIGNED_INT_VEC4								: return "uvec4";
			
			case GL_BOOL											: return "bool";
			case GL_BOOL_VEC2										: return "bvec2";
			case GL_BOOL_VEC3										: return "bvec3";
			case GL_BOOL_VEC4										: return "bvec4";
			
			/* Float matrices: */
			case GL_FLOAT_MAT2 										: return "mat2";
			case GL_FLOAT_MAT3 										: return "mat3";
			case GL_FLOAT_MAT4 										: return "mat4";
			
			case GL_FLOAT_MAT2x3 									: return "mat2x3";
			case GL_FLOAT_MAT2x4 									: return "mat2x4";
			case GL_FLOAT_MAT3x2 									: return "mat3x2";
			case GL_FLOAT_MAT3x4 									: return "mat3x4";
			case GL_FLOAT_MAT4x2 									: return "mat4x2";
			case GL_FLOAT_MAT4x3 									: return "mat4x3";

			/* Double matrices: */
			case GL_DOUBLE_MAT2 									: return "dmat2";
			case GL_DOUBLE_MAT3 									: return "dmat3";
			case GL_DOUBLE_MAT4 									: return "dmat4";
			
			case GL_DOUBLE_MAT2x3 									: return "dmat2x3";
			case GL_DOUBLE_MAT2x4 									: return "dmat2x4";
			case GL_DOUBLE_MAT3x2 									: return "dmat3x2";
			case GL_DOUBLE_MAT3x4 									: return "dmat3x4";
			case GL_DOUBLE_MAT4x2 									: return "dmat4x2";
			case GL_DOUBLE_MAT4x3 									: return "dmat4x3";

			/* Texture samplers: */
			case GL_SAMPLER_1D 										: return "sampler1D";
			case GL_SAMPLER_2D 										: return "sampler2D";
			case GL_SAMPLER_3D 										: return "sampler3D";

			case GL_SAMPLER_CUBE 									: return "samplerCube";
			
			case GL_SAMPLER_1D_SHADOW 								: return "sampler1DShadow";
			case GL_SAMPLER_2D_SHADOW 								: return "sampler2DShadow";
			
			case GL_SAMPLER_1D_ARRAY 								: return "sampler1DArray";
			case GL_SAMPLER_2D_ARRAY 								: return "sampler2DArray";
			
			case GL_SAMPLER_1D_ARRAY_SHADOW 						: return "sampler1DArrayShadow";
			case GL_SAMPLER_2D_ARRAY_SHADOW 						: return "sampler2DArrayShadow";
			
			case GL_SAMPLER_2D_MULTISAMPLE 							: return "sampler2DMS";
			
			case GL_SAMPLER_2D_MULTISAMPLE_ARRAY 					: return "sampler2DMSArray";
			
			case GL_SAMPLER_CUBE_SHADOW 							: return "samplerCubeShadow";
			
			case GL_SAMPLER_BUFFER 									: return "samplerBuffer";
			
			case GL_SAMPLER_2D_RECT 								: return "sampler2DRect";
			
			case GL_SAMPLER_2D_RECT_SHADOW 							: return "sampler2DRectShadow";
			
			case GL_INT_SAMPLER_1D 									: return "isampler1D";
			case GL_INT_SAMPLER_2D 									: return "isampler2D";
			case GL_INT_SAMPLER_3D 									: return "isampler3D";
			
			case GL_INT_SAMPLER_CUBE 								: return "isamplerCube";
			
			case GL_INT_SAMPLER_1D_ARRAY 							: return "isampler1DArray";
			case GL_INT_SAMPLER_2D_ARRAY 							: return "isampler2DArray";
			
			case GL_INT_SAMPLER_2D_MULTISAMPLE 						: return "isampler2DMS";
			
			case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 				: return "isampler2DMSArray";
			
			case GL_INT_SAMPLER_BUFFER 								: return "isamplerBuffer";
			
			case GL_INT_SAMPLER_2D_RECT 							: return "isampler2DRect";
			
			case GL_UNSIGNED_INT_SAMPLER_1D 						: return "usampler1D";
			case GL_UNSIGNED_INT_SAMPLER_2D 						: return "usampler2D";
			case GL_UNSIGNED_INT_SAMPLER_3D 						: return "usampler3D";
			
			case GL_UNSIGNED_INT_SAMPLER_CUBE 						: return "usamplerCube";
			
			case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY 					: return "usampler2DArray";
			case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY 					: return "usampler2DArray";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE 			: return "usampler2DMS";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY 		: return "usampler2DMSArray";
			
			case GL_UNSIGNED_INT_SAMPLER_BUFFER 					: return "usamplerBuffer";
			
			case GL_UNSIGNED_INT_SAMPLER_2D_RECT 					: return "usampler2DRect";
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::NameOf() called with an unknown GL type!" );
	}

	header_function const GLenum TypeOf( const char* name )
	{
		local_persist std::unordered_map< std::string, GLenum > lookUp_table =
		{
			/* Scalars & vectors: */
			{ "float",					GL_FLOAT },
			{ "vec2",					GL_FLOAT_VEC2 },
			{ "vec3",					GL_FLOAT_VEC3 },
			{ "vec4",					GL_FLOAT_VEC4 },
			
			{ "double",					GL_DOUBLE },
			{ "dvec2",					GL_DOUBLE_VEC2 },
			{ "dvec3",					GL_DOUBLE_VEC3 },
			{ "dvec4",					GL_DOUBLE_VEC4 },

			{ "int",					GL_INT },
			{ "ivec2",					GL_INT_VEC2 },
			{ "ivec3",					GL_INT_VEC3 },
			{ "ivec4",					GL_INT_VEC4 },
			
			{ "unsigned int",			GL_UNSIGNED_INT },
			{ "uvec2",					GL_UNSIGNED_INT_VEC2 },
			{ "uvec3",					GL_UNSIGNED_INT_VEC3 },
			{ "uvec4",					GL_UNSIGNED_INT_VEC4 },
			
			{ "bool",					GL_BOOL },
			{ "bvec2",					GL_BOOL_VEC2 },
			{ "bvec3",					GL_BOOL_VEC3 },
			{ "bvec4",					GL_BOOL_VEC4 },
			
			/* Float matrices: */
			{ "mat2",					GL_FLOAT_MAT2 },
			{ "mat3",					GL_FLOAT_MAT3 },
			{ "mat4",					GL_FLOAT_MAT4 },
			
			{ "mat2x3",					GL_FLOAT_MAT2x3 },
			{ "mat2x4",					GL_FLOAT_MAT2x4 },
			{ "mat3x2",					GL_FLOAT_MAT3x2 },
			{ "mat3x4",					GL_FLOAT_MAT3x4 },
			{ "mat4x2",					GL_FLOAT_MAT4x2 },
			{ "mat4x3",					GL_FLOAT_MAT4x3 },

			/* Double matrices: */
			{ "dmat2",					GL_DOUBLE_MAT2 },
			{ "dmat3",					GL_DOUBLE_MAT3 },
			{ "dmat4",					GL_DOUBLE_MAT4 },
			
			{ "dmat2x3",				GL_DOUBLE_MAT2x3 },
			{ "dmat2x4",				GL_DOUBLE_MAT2x4 },
			{ "dmat3x2",				GL_DOUBLE_MAT3x2 },
			{ "dmat3x4",				GL_DOUBLE_MAT3x4 },
			{ "dmat4x2",				GL_DOUBLE_MAT4x2 },
			{ "dmat4x3",				GL_DOUBLE_MAT4x3 },

			/* Texture samplers: */
			{ "sampler1D",				GL_SAMPLER_1D },
			{ "sampler2D",				GL_SAMPLER_2D },
			{ "sampler3D",				GL_SAMPLER_3D },

			{ "samplerCube",			GL_SAMPLER_CUBE },
			
			{ "sampler1DShadow",		GL_SAMPLER_1D_SHADOW },
			{ "sampler2DShadow",		GL_SAMPLER_2D_SHADOW },
			
			{ "sampler1DArray",			GL_SAMPLER_1D_ARRAY },
			{ "sampler2DArray",			GL_SAMPLER_2D_ARRAY },
			
			{ "sampler1DArrayShadow",	GL_SAMPLER_1D_ARRAY_SHADOW },
			{ "sampler2DArrayShadow",	GL_SAMPLER_2D_ARRAY_SHADOW },
			
			{ "sampler2DMS",			GL_SAMPLER_2D_MULTISAMPLE },
			
			{ "sampler2DMSArray",		GL_SAMPLER_2D_MULTISAMPLE_ARRAY },
			
			{ "samplerCubeShadow",		GL_SAMPLER_CUBE_SHADOW },
			
			{ "samplerBuffer",			GL_SAMPLER_BUFFER },
			
			{ "sampler2DRect",			GL_SAMPLER_2D_RECT },
			
			{ "sampler2DRectShadow",	GL_SAMPLER_2D_RECT_SHADOW },
			
			{ "isampler1D",				GL_INT_SAMPLER_1D },
			{ "isampler2D",				GL_INT_SAMPLER_2D },
			{ "isampler3D",				GL_INT_SAMPLER_3D },
			
			{ "isamplerCube",			GL_INT_SAMPLER_CUBE },
			
			{ "isampler1DArray",		GL_INT_SAMPLER_1D_ARRAY },
			{ "isampler2DArray",		GL_INT_SAMPLER_2D_ARRAY },
			
			{ "isampler2DMS",			GL_INT_SAMPLER_2D_MULTISAMPLE },
			
			{ "isampler2DMSArray",		GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY },
			
			{ "isamplerBuffer",			GL_INT_SAMPLER_BUFFER },
			
			{ "isampler2DRect",			GL_INT_SAMPLER_2D_RECT },
			
			{ "usampler1D",				GL_UNSIGNED_INT_SAMPLER_1D },
			{ "usampler2D",				GL_UNSIGNED_INT_SAMPLER_2D },
			{ "usampler3D",				GL_UNSIGNED_INT_SAMPLER_3D },
			
			{ "usamplerCube",			GL_UNSIGNED_INT_SAMPLER_CUBE },
			
			{ "usampler2DArray",		GL_UNSIGNED_INT_SAMPLER_1D_ARRAY },
			{ "usampler2DArray",		GL_UNSIGNED_INT_SAMPLER_2D_ARRAY },
			
			{ "usampler2DMS",			GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE },
			
			{ "usampler2DMSArray",		GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY },
			
			{ "usamplerBuffer",			GL_UNSIGNED_INT_SAMPLER_BUFFER },
			
			{ "usampler2DRect",			GL_UNSIGNED_INT_SAMPLER_2D_RECT },
		};

		return lookUp_table[ name ];
	}

	header_function GLenum ComponentTypeOf( const GLenum type )
	{
		switch( type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT				:
			case GL_FLOAT_VEC2			:
			case GL_FLOAT_VEC3			:
			case GL_FLOAT_VEC4			: return GL_FLOAT;

			case GL_DOUBLE				:
			case GL_DOUBLE_VEC2			:
			case GL_DOUBLE_VEC3			:
			case GL_DOUBLE_VEC4			: return GL_DOUBLE;

			case GL_INT					:
			case GL_INT_VEC2			:
			case GL_INT_VEC3			:
			case GL_INT_VEC4			: return GL_INT;

			case GL_UNSIGNED_INT		:
			case GL_UNSIGNED_INT_VEC2	:
			case GL_UNSIGNED_INT_VEC3	:
			case GL_UNSIGNED_INT_VEC4	: return GL_UNSIGNED_INT;

			case GL_BOOL				:
			case GL_BOOL_VEC2			:
			case GL_BOOL_VEC3			:
			case GL_BOOL_VEC4			: return GL_BOOL;

			/* Float matrices: */
			case GL_FLOAT_MAT2 			:
			case GL_FLOAT_MAT3 			:
			case GL_FLOAT_MAT4 			:

			case GL_FLOAT_MAT2x3 		:
			case GL_FLOAT_MAT2x4 		:
			case GL_FLOAT_MAT3x2 		:
			case GL_FLOAT_MAT3x4 		:
			case GL_FLOAT_MAT4x2 		:
			case GL_FLOAT_MAT4x3 		: return GL_FLOAT;

			/* Double matrices: */
			case GL_DOUBLE_MAT2 		:
			case GL_DOUBLE_MAT3 		:
			case GL_DOUBLE_MAT4 		:

			case GL_DOUBLE_MAT2x3 		:
			case GL_DOUBLE_MAT2x4 		:
			case GL_DOUBLE_MAT3x2 		:
			case GL_DOUBLE_MAT3x4 		:
			case GL_DOUBLE_MAT4x2 		:
			case GL_DOUBLE_MAT4x3 		: return GL_DOUBLE;
		}

		throw std::runtime_error( "ERROR::SHADER_TYPE::ComponentTypeOf() called with an unknown GL type!" );
	}

	header_function void* AddressOf( const GLenum type, void* address, const int element_index )
	{
		return ( void* )( ( uintptr_t )address + element_index * SizeOf( type ) );
	}

	header_function const void* AddressOf( const GLenum type, const void* address, const int element_index )
	{
		return ( const void* )( ( uintptr_t )address + element_index * SizeOf( type ) );
	}
}
