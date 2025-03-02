#pragma once

// Engine Includes.
#include "Graphics.h"
#include "Color.hpp"
#include "Id.hpp"
#include "ShaderSourcePath.hpp"
#include "Std140StructTag.h"
#include "Uniform.h"
#include "VertexLayout.hpp"
#include "Math/Concepts.h"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

// std Includes.
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace Engine
{
	/* Forward Declarations: */
	class Renderer;
	class BuiltinShaders;

	class Shader
	{
		friend class Renderer;
		friend class BuiltinShaders;
		friend class std::unordered_map< std::string, Shader >;

		using ReferenceCount = unsigned int;

	public:
		using ID       = ID< Renderer >;
		using Features = std::vector< std::string >;

		/* Features can be:
		 *		Declared in shaders, via "#pragma feature <feature_name>" syntax. 
					These types of Features need to be set from the client side to be defined by the Shader class before compilation (i.e., source is modified to #define the Feature).
		 *		Defined in shaders directly, via "#define <feature_name> <optional_value>" syntax.
		 */
		struct Feature
		{
			std::optional< std::string > value;
			bool is_set; // This means that the feature was either directly #define'd or is set by client code & will effectively be #define'd in the final shader source.
			// bool padding[ 7 ];
		};

	public:
		/* Will be initialized later with FromFile(). */
		Shader( const char* name );
		Shader( const char* name,
				const VertexShaderSourcePath& vertex_shader_source_path,
				const FragmentShaderSourcePath& fragment_shader_source_path, 
				const Features& features_to_set = {} );
		Shader( const char* name,
				const VertexShaderSourcePath& vertex_shader_source_path,
				const GeometryShaderSourcePath& geometry_shader_source_path,
				const FragmentShaderSourcePath& fragment_shader_source_path,
				const Features& features_to_set = {} );

		DELETE_COPY_CONSTRUCTORS( Shader );

		Shader( Shader&& );
		Shader& operator=( Shader&& );

		~Shader();

/* Usage: */

		void Bind() const;

/* Compilation: */

		bool FromFile( const VertexShaderSourcePath& vertex_shader_source_path,
					   const FragmentShaderSourcePath& fragment_shader_source_path,
					   const Features& features_to_set = {} );
		bool FromFile( const VertexShaderSourcePath& vertex_shader_source_path,
					   const GeometryShaderSourcePath& geometry_shader_source_path,
					   const FragmentShaderSourcePath& fragment_shader_source_path,
					   const Features& features_to_set = {} );


		bool RecompileFromThis( Shader& new_shader );

/* Queries: */

		inline		 ID								Id()							const { return program_id;							}
		inline const std::string&					Name()							const { return name;								}
		inline bool									HasGeometryStage()				const { return not geometry_source_path.empty();	}
		inline const std::string&					VertexSourcePath()				const { return vertex_source_path;					}
		inline const std::string&					GeometrySourcePath()			const { return geometry_source_path;				}
		inline const std::string&					FragmentSourcePath()			const { return fragment_source_path;				}
		inline const std::vector< std::string >&	VertexSourceIncludePaths()		const { return vertex_source_include_path_array;	}
		inline const std::vector< std::string >&	GeometrySourceIncludePaths()	const { return geometry_source_include_path_array;	}
		inline const std::vector< std::string >&	FragmentSourceIncludePaths()	const { return fragment_source_include_path_array;	}

		inline const std::unordered_map< std::string, Feature >& GetFeatures() const { return feature_map; }

		inline const VertexLayout& GetSourceVertexLayout()	const { return vertex_layout_source; }
		inline const VertexLayout& GetActiveVertexLayout()	const { return vertex_layout_active; }

		bool SourceFilesAreModified();

/* Uniform APIs: */

		inline const std::unordered_map< std::string, Uniform::Information >& GetUniformInfoMap() const { return uniform_info_map; }

		inline const std::unordered_map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap_Regular()	const { return uniform_buffer_info_map_regular;		}
		inline const std::unordered_map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap_Global()		const { return uniform_buffer_info_map_global;		}
		inline const std::unordered_map< std::string, Uniform::BufferInformation	>& GetUniformBufferInfoMap_Intrinsic()	const { return uniform_buffer_info_map_intrinsic;	}

		inline std::size_t GetTotalUniformSize_DefaultBlockOnly()	const { return uniform_book_keeping_info.default_block_size;			}
		inline std::size_t GetTotalUniformSize_UniformBlocksOnly()	const { return uniform_book_keeping_info.TotalSize_Blocks();			}
		inline std::size_t GetTotalUniformSize_ForMaterial()		const { return uniform_book_keeping_info.TotalSize_ForMaterialBlob();	}
		inline std::size_t GetTotalUniformSize()					const { return uniform_book_keeping_info.total_size;					}

		inline bool HasDefaultUniforms()		const { return uniform_book_keeping_info.default_block_size > 0;	}
		inline bool HasIntrinsicUniformBlocks()	const { return uniform_book_keeping_info.intrinsic_block_count;		}
		inline bool HasGlobalUniformBlocks()	const { return uniform_book_keeping_info.global_block_count;		}
		inline bool HasRegularUniformBlocks()	const { return uniform_book_keeping_info.regular_block_count;		}
		
		inline bool HasUniformBlocks() const { return HasIntrinsicUniformBlocks() || HasGlobalUniformBlocks() || HasRegularUniformBlocks(); }

		inline bool HasIntrinsicUniformBlock( const std::string& block_name )	const { return uniform_buffer_info_map_intrinsic.contains( block_name ); }
		inline bool HasGlobalUniformBlock( const std::string& block_name )		const { return uniform_buffer_info_map_global.contains( block_name ); }
		inline bool HasRegularUniformBlock( const std::string& block_name )		const { return uniform_buffer_info_map_regular.contains( block_name ); }

/* Uniform Upload; Non-array types: */

		template< typename UniformType >
		void SetUniform( const int location, const UniformType& value );

		template<>
		void SetUniform< float >( const int location, const float& value )
		{
			glUniform1f( location, value );
		}

		template<>
		void SetUniform< int >( const int location, const int& value )
		{
			glUniform1i( location, value );
		}

		template<>
		void SetUniform< unsigned int >( const int location, const unsigned int& value )
		{
			glUniform1ui( location, value );
		}

		template<>
		void SetUniform< bool >( const int location, const bool& value )
		{
			glUniform1i( location, value );
		}

		/* Vectors. */
		template< typename Component, std::size_t Size > requires( Size >= 2 && Size <= 4 )
		void SetUniform( const int location, const Math::Vector< Component, Size >& value )
		{
			if constexpr( std::is_same_v< Component, bool > )
			{
				/* Since sizeof( bool ) != sizeof( int ), we can not just pass a pointer to data and call the v variant of glUniformX functions. */

				if constexpr( Size == 2 )
				{
					glUniform2i( location, value.X(), value.Y() );
				}
				if constexpr( Size == 3 )
				{
					glUniform3i( location, value.X(), value.Y(), value.Z() );
				}
				if constexpr( Size == 4 )
				{
					glUniform4i( location, value.X(), value.Y(), value.Z(), value.W() );
				}
			}
			else
			{
				if constexpr( std::is_same_v< Component, float > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 3 )
					{
						glUniform3fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 4 )
					{
						glUniform4fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
				}

				if constexpr( std::is_same_v< Component, int > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2iv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 3 )
					{
						glUniform3iv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 4 )
					{
						glUniform4iv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
				}

				if constexpr( std::is_same_v< Component, unsigned int > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2uiv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 3 )
					{
						glUniform3uiv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
					if constexpr( Size == 4 )
					{
						glUniform4uiv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, value.Data() );
					}
				}
			}
		}

		template<>
		void SetUniform< Color3 >( const int location, const Engine::Color3& value )
		{
			SetUniform( location, reinterpret_cast< const Vector3& >( value ) );
		}

		template<>
		void SetUniform< Color4 >( const int location, const Engine::Color4& value )
		{
			SetUniform( location, reinterpret_cast< const Vector4& >( value ) );
		}

		template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize > && Concepts::NonZero< ColumnSize >
		void SetUniform( const int location, const Math:: Matrix< Type, RowSize, ColumnSize >& value )
		{
			if constexpr( RowSize == ColumnSize )
			{
				if constexpr( RowSize == 2U )
				{
					glUniformMatrix2fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 3U )
				{
					glUniformMatrix3fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 4U )
				{
					glUniformMatrix4fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
			}
			else
			{
				if constexpr( RowSize == 2U && ColumnSize == 3U )
				{
					glUniformMatrix2x3fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 2U && ColumnSize == 4U )
				{
					glUniformMatrix2x4fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 3U && ColumnSize == 2U )
				{
					glUniformMatrix3x2fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 3U && ColumnSize == 4U )
				{
					glUniformMatrix3x4fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 4U && ColumnSize == 2U )
				{
					glUniformMatrix4x2fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
				if constexpr( RowSize == 4U && ColumnSize == 3U )
				{
					glUniformMatrix4x3fv( location, /* This is the number of ARRAY elements, which is 1 for non-arrays */ 1, GL_TRUE, value.Data() );
				}
			}
		}

/* Uniform Upload; Array types: */

		template< typename UniformType >
		void SetUniformArray( const int location, const UniformType* address, const int element_count );

		template<>
		void SetUniformArray< float >( const int location, const float* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );
			glUniform1fv( location, element_count, address );
		}

		template<>
		void SetUniformArray< int >( const int location, const int* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );
			glUniform1iv( location, element_count, address );
		}

		template<>
		void SetUniformArray< unsigned int >( const int location, const unsigned int* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );
			glUniform1uiv( location, element_count, address );
		}

		template<>
		void SetUniformArray< bool >( const int location, const bool* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );
			glUniform1iv( location, element_count, reinterpret_cast< const int* >( address ) );
		}

		/* Vectors. */
		template< typename Component, std::size_t Size > requires( Size >= 2 && Size <= 4 )
		void SetUniformArray( const int location, const Math::Vector< Component, Size >* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );
			if constexpr( std::is_same_v< Component, bool > )
			{
				/* Since sizeof( bool ) != sizeof( int ), we have to create a Vector<> of integers, copy the data into it & pass its address for the memory to be laid out correctly: */
				Math::Vector< int, Size > integer_version;
				for( int i = 0; i < Size; i++ )
					integer_version[ i ] = ( *address )[ i ];

				if constexpr( Size == 2 )
				{
					glUniform2iv( location, element_count, reinterpret_cast< const int* >( integer_version.Data() ) );
				}
				if constexpr( Size == 3 )
				{
					glUniform3iv( location, element_count, reinterpret_cast< const int* >( integer_version.Data() ) );
				}
				if constexpr( Size == 4 )
				{
					glUniform4iv( location, element_count, reinterpret_cast< const int* >( integer_version.Data() ) );
				}
			}
			else
			{
				if constexpr( std::is_same_v< Component, float > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2fv( location, element_count, reinterpret_cast< const float* >( address ) );
					}
					if constexpr( Size == 3 )
					{
						glUniform3fv( location, element_count, reinterpret_cast< const float* >( address ) );
					}
					if constexpr( Size == 4 )
					{
						glUniform4fv( location, element_count, reinterpret_cast< const float* >( address ) );
					}
				}

				if constexpr( std::is_same_v< Component, int > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2iv( location, element_count, reinterpret_cast< const int* >( address ) );
					}
					if constexpr( Size == 3 )
					{
						glUniform3iv( location, element_count, reinterpret_cast< const int* >( address ) );
					}
					if constexpr( Size == 4 )
					{
						glUniform4iv( location, element_count, reinterpret_cast< const int* >( address ) );
					}
				}

				if constexpr( std::is_same_v< Component, unsigned int > )
				{
					if constexpr( Size == 2 )
					{
						glUniform2uiv( location, element_count, reinterpret_cast< const unsigned int* >( address ) );
					}
					if constexpr( Size == 3 )
					{
						glUniform3uiv( location, element_count, reinterpret_cast< const unsigned int* >( address ) );
					}
					if constexpr( Size == 4 )
					{
						glUniform4uiv( location, element_count, reinterpret_cast< const unsigned int* >( address ) );
					}
				}
			}
		}

		template<>
		void SetUniformArray< Color3 >( const int location, const Engine::Color3* address, const int element_count )
		{
			SetUniformArray( location, reinterpret_cast< const Vector3* >( address ), element_count );
		}

		template<>
		void SetUniformArray< Color4 >( const int location, const Engine::Color4* address, const int element_count )
		{
			SetUniformArray( location, reinterpret_cast< const Vector4* >( address ), element_count );
		}

		template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize > && Concepts::NonZero< ColumnSize >
		void SetUniformArray( const int location, const Math:: Matrix< Type, RowSize, ColumnSize >* address, const int element_count )
		{
			ASSERT_DEBUG_ONLY( element_count > 1 );

			if constexpr( RowSize == ColumnSize )
			{
				if constexpr( RowSize == 2U )
				{
					glUniformMatrix2fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 3U )
				{
					glUniformMatrix3fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 4U )
				{
					glUniformMatrix4fv( location, element_count, GL_TRUE, address->Data() );
				}
			}
			else
			{
				if constexpr( RowSize == 2U && ColumnSize == 3U )
				{
					glUniformMatrix2x3fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 2U && ColumnSize == 4U )
				{
					glUniformMatrix2x4fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 3U && ColumnSize == 2U )
				{
					glUniformMatrix3x2fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 3U && ColumnSize == 4U )
				{
					glUniformMatrix3x4fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 4U && ColumnSize == 2U )
				{
					glUniformMatrix4x2fv( location, element_count, GL_TRUE, address->Data() );
				}
				if constexpr( RowSize == 4U && ColumnSize == 3U )
				{
					glUniformMatrix4x3fv( location, element_count, GL_TRUE, address->Data() );
				}
			}
		}

	/*
	 * "Setting" Uniform Buffers is not the Shader's responsibility (nor can it do it); It's just an OpenGL Buffer Object.
	 * UBO can Upload() itself.
	 * Whoever holds the UBO will call Upload() on it.
	 * All UBOs are created, connected to shader blocks and kept by the UniformBufferManager internally.
	 * UniformBufferManagement objects hold references to buffers kept by the UniformBufferManager and manage them (keep CPU-side buffers large enough & update/upload buffers).
	 * Global & Intrinsic Uniform Buffers are updated by the Renderer (via UniformBufferManagement objects).
	 * Regular			  Uniform Buffers are updated by the Material (via an UniformBufferManagement object).
	 */

/* Uniform setters; By name & value: */

		template< typename UniformType >
		/* Prohibit Uniform Buffers: */ requires( not std::is_base_of_v< Std140StructTag, UniformType > )
		void SetUniform( const char* uniform_name, const UniformType& value )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			SetUniform( uniform_info.location_or_block_index, value );
		}

		template< typename UniformType >
		/* Prohibit Uniform Buffers: */ requires( not std::is_base_of_v< Std140StructTag, UniformType > )
			void SetUniformArray( const char* uniform_name, const UniformType* value, const int element_count )
		{
			const auto& uniform_info = GetUniformInformation( uniform_name );

			SetUniformArray( uniform_info.location_or_block_index, value, element_count );
		}

/* Uniform setters; By info. & pointer: */

		void SetUniform( const Uniform::Information& uniform_info, const void* value_pointer );
		void SetUniformArray( const Uniform::Information& uniform_info, const void* value_pointer );



	private:



		void Delete();

/* Queries: */

		inline bool IsValid() const { return program_id.IsValid(); }

/* Compilation & Linkage: */

		std::optional< std::string > ParseShaderFromFile( const char* file_path, const ShaderType shader_type );
		std::vector< std::string > PreprocessShaderStage_GetIncludeFilePaths( std::string shader_source ) const;
		void PreprocessShaderStage_StripDefinesToBeSet( std::string& shader_source_to_modify, const std::vector< std::string >& features_to_set );
		std::unordered_map< std::string, Feature > PreProcessShaderStage_ParseFeatures( std::string shader_source );
		void PreProcessShaderStage_SetFeatures( std::string& shader_source_to_modify,
												std::unordered_map< std::string, Feature >& defined_features,
												const std::vector< std::string >& features_to_set );
		bool PreProcessShaderStage_IncludeDirectives( const std::filesystem::path& shader_source_path, std::string& shader_source_to_modify, const ShaderType shader_type );
		bool CompileShader( const char* source, unsigned int& shader_id, const ShaderType shader_type );
		bool LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id );
		bool LinkProgram( const unsigned int vertex_shader_id, const unsigned int geometry_shader_id, const unsigned int fragment_shader_id );

		/*std::string ShaderSource_CommentsStripped( const std::string& shader_source );*/
		void ParseShaderSource_UniformUsageHints( const std::string& shader_source, const ShaderType shader_type );
		void ParseShaderSource_VertexLayout( std::string shader_source );

/* Shader Introspection: */

		void QueryVertexAttributes();

		void GetUniformBookKeepingInfo();
		
		/* Expects empty input vectors. */
		bool GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																		 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const;
		void QueryUniformData();
		void QueryUniformData_BlockIndexAndOffsetForBufferMembers();
		void QueryUniformBufferData( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map, const Uniform::BufferCategory category );
		void QueryUniformBufferData_Aggregates( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map );
		void CalculateTotalUniformSizes();
		void EnumerateUniformBufferCategories();

		const Uniform::Information& GetUniformInformation( const std::string& uniform_name );

/* Error Checking/Reporting: */

		void LogErrors( const std::string& error_string ) const;
		void LogErrors_Compilation( const int shader_id, const ShaderType shader_type ) const;
		void LogErrors_Linking() const;
		std::string FormatErrorLog( const char* log ) const;

/* Editor: */

		std::string UniformEditorName( const std::string_view original_name );
		std::string UniformEditorName_BufferMemberAggregate( const std::string_view aggregate_name );

	private:
		ID program_id;
		//int padding;
		std::string name;

		std::string vertex_source_path;
		std::string geometry_source_path;
		std::string fragment_source_path;

		std::vector< std::string > vertex_source_include_path_array;
		std::vector< std::string > geometry_source_include_path_array;
		std::vector< std::string > fragment_source_include_path_array;

		std::vector< std::string > features_requested;
		std::unordered_map< std::string, Feature > feature_map;

		std::unordered_map< std::string, Uniform::Information > uniform_info_map;

		std::unordered_map< std::string, Uniform::BufferInformation	> uniform_buffer_info_map_regular;
		std::unordered_map< std::string, Uniform::BufferInformation	> uniform_buffer_info_map_global;
		std::unordered_map< std::string, Uniform::BufferInformation	> uniform_buffer_info_map_intrinsic;

		std::unordered_map< std::string, std::filesystem::file_time_type > last_write_time_map;

		Uniform::ActiveUniformBookKeepingInformation uniform_book_keeping_info;

		VertexLayout vertex_layout_source;
		VertexLayout vertex_layout_active;
	};
}