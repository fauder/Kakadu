// Platform-specific Debug API includes.
#if defined( _WIN32 ) && defined( _DEBUG )
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engince Includes.
#include "Asset/Paths.h"
#include "Core/BitFlags.hpp"
#include "Core/ServiceLocator.h"
#include "Core/Utility.hpp"
#include "GLLogger.h"
#include "GraphicsDeviceInfo.h"
#include "Shader.hpp"
#include "ShaderIncludePreprocessing.h"
#include "ShaderTypeInformation.h"
#include "UniformBlockBindingPointManager.h"

// std Includes.
#include <numeric> // std::iota.
#include <charconv>

namespace Engine
{
	/* Will be initialized later with FromFile(). */
	Shader::Shader( const char* name )
		:
		program_id( 0 ),
		name( name )
	{
	}

	Shader::Shader( const char* name,
					const VertexShaderSourcePath& vertex_shader_source_path,
					const FragmentShaderSourcePath& fragment_shader_source_path,
					const Features& features_to_set )
		:
		name( name ),
		vertex_source_path( vertex_shader_source_path ),
		fragment_source_path( fragment_shader_source_path ),
		features_requested( features_to_set.begin(), features_to_set.end() )
	{
		FromFile( vertex_shader_source_path, fragment_shader_source_path, features_to_set );
	}

	Shader::Shader( const char* name,
					const VertexShaderSourcePath& vertex_shader_source_path,
					const GeometryShaderSourcePath& geometry_shader_source_path,
					const FragmentShaderSourcePath& fragment_shader_source_path,
					const Features& features_to_set )
		:
		name( name ),
		vertex_source_path( vertex_shader_source_path ),
		geometry_source_path( geometry_shader_source_path ),
		fragment_source_path( fragment_shader_source_path ),
		features_requested( features_to_set )
	{
		FromFile( vertex_shader_source_path, geometry_shader_source_path, fragment_shader_source_path, features_to_set );
	}

	Shader::Shader( Shader&& donor )
		:
		name( std::exchange( donor.name, "<scheduled-for-deletion>" ) ),

		vertex_source_path( std::move( donor.vertex_source_path ) ),
		geometry_source_path( std::move( donor.geometry_source_path ) ),
		fragment_source_path( std::move( donor.fragment_source_path ) ),

		vertex_source_include_path_array( std::move( donor.vertex_source_include_path_array ) ),
		geometry_source_include_path_array( std::move( donor.geometry_source_include_path_array ) ),
		fragment_source_include_path_array( std::move( donor.fragment_source_include_path_array ) ),

		features_requested( std::move( donor.features_requested ) ),
		feature_map( std::move( donor.feature_map ) ),

		uniform_info_map( std::move( donor.uniform_info_map ) ),

		uniform_buffer_info_map_regular( std::move( donor.uniform_buffer_info_map_regular ) ),
		uniform_buffer_info_map_global( std::move( donor.uniform_buffer_info_map_global ) ),
		uniform_buffer_info_map_intrinsic( std::move( donor.uniform_buffer_info_map_intrinsic ) ),

		last_write_time_map( std::exchange( donor.last_write_time_map, {} ) ),

		uniform_book_keeping_info( std::move( donor.uniform_book_keeping_info ) ),

		uniform_annotation_format_string_table( std::move( donor.uniform_annotation_format_string_table ) ),

		vertex_layout_source( std::move( donor.vertex_layout_source ) ),
		vertex_layout_active( std::move( donor.vertex_layout_active ) )
	{
		Delete();

		program_id = std::exchange( donor.program_id, {} );
	}

	Shader& Shader::operator=( Shader&& donor )
	{
		Delete();

		program_id = std::exchange( donor.program_id, {} );
		name       = std::exchange( donor.name, "<scheduled-for-deletion>" );

		vertex_source_path   = std::move( donor.vertex_source_path );
		geometry_source_path = std::move( donor.geometry_source_path );
		fragment_source_path = std::move( donor.fragment_source_path );

		vertex_source_include_path_array   = std::move( donor.vertex_source_include_path_array );
		geometry_source_include_path_array = std::move( donor.geometry_source_include_path_array );
		fragment_source_include_path_array = std::move( donor.fragment_source_include_path_array );

		features_requested = std::move( donor.features_requested );
		feature_map        = std::move( donor.feature_map );

		uniform_info_map = std::move( donor.uniform_info_map );

		uniform_buffer_info_map_regular   = std::move( donor.uniform_buffer_info_map_regular );
		uniform_buffer_info_map_global    = std::move( donor.uniform_buffer_info_map_global );
		uniform_buffer_info_map_intrinsic = std::move( donor.uniform_buffer_info_map_intrinsic );

		last_write_time_map = std::exchange( donor.last_write_time_map, {} );

		uniform_book_keeping_info = std::move( donor.uniform_book_keeping_info );

		uniform_annotation_format_string_table = std::move( donor.uniform_annotation_format_string_table );

		vertex_layout_source = std::move( donor.vertex_layout_source );
		vertex_layout_active = std::move( donor.vertex_layout_active );

		return *this;
	}

	Shader::~Shader()
	{
		Delete();
	}

	bool Shader::FromFile( const VertexShaderSourcePath& vertex_shader_source_path,
						   const FragmentShaderSourcePath& fragment_shader_source_path,
						   const Features& features_to_set )
	{
		using namespace Literals;

		return FromFile( vertex_shader_source_path, ""_geom, fragment_shader_source_path, features_to_set );
	}

	bool Shader::FromFile( const VertexShaderSourcePath& vertex_shader_source_path,
						   const GeometryShaderSourcePath& geometry_shader_source_path,
						   const FragmentShaderSourcePath& fragment_shader_source_path,
						   const Features& features_to_set )
	{
		this->vertex_source_path   = ( std::string )vertex_shader_source_path;
		this->geometry_source_path = ( std::string )geometry_shader_source_path;
		this->fragment_source_path = ( std::string )fragment_shader_source_path;

		features_requested = features_to_set;

		/* Even though the shader may fail the following compilation or linking stages, last write time should be set in order to make recompilation (due to user modification of sources) possible. */
		auto SaveTimeIfValid = [ & ]( auto& path )
		{
			std::error_code error_code;
			auto t = std::filesystem::last_write_time( path, error_code );
			if( error_code )
				std::cerr << "ERROR::SHADER::COMPILATION::LAST_WRITE_TIME_COULD_NOT_BE_OBTAINED\n\t" << error_code.message() << "\n";
			else
				last_write_time_map.emplace( path, t );
		};

		SaveTimeIfValid( vertex_source_path );
		if( not geometry_shader_source_path.Empty() )
			SaveTimeIfValid( geometry_source_path );
		SaveTimeIfValid( fragment_source_path );

		unsigned int vertex_shader_id = 0, geometry_shader_id = 0, fragment_shader_id = 0;

		std::optional< std::string > vertex_shader_source;
		std::optional< std::string > geometry_shader_source;
		std::optional< std::string > fragment_shader_source;
		std::unordered_map< std::string, Feature > vertex_shader_features;
		std::unordered_map< std::string, Feature > geometry_shader_features;
		std::unordered_map< std::string, Feature > fragment_shader_features;

		if( vertex_shader_source = ParseShaderFromFile( vertex_shader_source_path, ShaderType::Vertex );
			vertex_shader_source )
		{
			auto& shader_source = *vertex_shader_source;

			std::unordered_map< std::int16_t, std::filesystem::path > map_of_IDs_per_include_file;

			vertex_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
			PreProcessShaderStage_IncludeDirectives( vertex_shader_source_path, shader_source, ShaderType::Vertex, map_of_IDs_per_include_file );
			vertex_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
			PreProcessShaderStage_SetFeatures( shader_source, vertex_shader_features, features_to_set );

			if( !CompileShader( shader_source.c_str(), vertex_shader_id, ShaderType::Vertex, map_of_IDs_per_include_file ) )
				return false;
		}
		else
			return false;

#ifdef _EDITOR
		ServiceLocator< GLLogger >::Get().SetLabel( GL_SHADER, vertex_shader_id, GL_LABEL_PREFIX_VERTEX_SHADER + name );
#endif // _EDITOR

		feature_map.insert( vertex_shader_features.begin(), vertex_shader_features.end() );

		if( not geometry_shader_source_path.Empty() )
		{
			if( geometry_shader_source = ParseShaderFromFile( geometry_shader_source_path, ShaderType::Geometry );
				geometry_shader_source )
			{
				auto& shader_source = *geometry_shader_source;

				std::unordered_map< std::int16_t, std::filesystem::path > map_of_IDs_per_include_file;

				geometry_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
				PreProcessShaderStage_IncludeDirectives( geometry_shader_source_path, shader_source, ShaderType::Geometry, map_of_IDs_per_include_file );
				geometry_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
				PreProcessShaderStage_SetFeatures( shader_source, geometry_shader_features, features_to_set );

				if( !CompileShader( shader_source.c_str(), geometry_shader_id, ShaderType::Geometry, map_of_IDs_per_include_file ) )
				{
					glDeleteShader( vertex_shader_id );
					return false;
				}
			}
			else
			{
				glDeleteShader( vertex_shader_id );
				return false;
			}

			feature_map.insert( geometry_shader_features.begin(), geometry_shader_features.end() );

#ifdef _EDITOR
				ServiceLocator< GLLogger >::Get().SetLabel( GL_SHADER, geometry_shader_id, GL_LABEL_PREFIX_GEOMETRY_SHADER + name );
#endif // _EDITOR
		}

		if( fragment_shader_source = ParseShaderFromFile( fragment_shader_source_path, ShaderType::Fragment );
			fragment_shader_source )
		{
			auto& shader_source = *fragment_shader_source;

			std::unordered_map< std::int16_t, std::filesystem::path > map_of_IDs_per_include_file;

			fragment_source_include_path_array = PreprocessShaderStage_GetIncludeFilePaths( shader_source );
			PreProcessShaderStage_IncludeDirectives( fragment_shader_source_path, shader_source, ShaderType::Fragment, map_of_IDs_per_include_file );
			fragment_shader_features = PreProcessShaderStage_ParseFeatures( shader_source );
			PreProcessShaderStage_SetFeatures( shader_source, fragment_shader_features, features_to_set );

			if( !CompileShader( shader_source.c_str(), fragment_shader_id, ShaderType::Fragment, map_of_IDs_per_include_file ) )
			{
				glDeleteShader( vertex_shader_id );
				if( geometry_shader_id > 0 )
					glDeleteShader( geometry_shader_id );
				return false;
			}
		}
		else
		{
			glDeleteShader( vertex_shader_id );
			if( geometry_shader_id > 0 )
				glDeleteShader( geometry_shader_id );

			return false;
		}

#ifdef _EDITOR
		ServiceLocator< GLLogger >::Get().SetLabel( GL_SHADER, fragment_shader_id, GL_LABEL_PREFIX_FRAGMENT_SHADER + name );
#endif // _EDITOR

		feature_map.insert( fragment_shader_features.begin(), fragment_shader_features.end() );

		const bool link_result = LinkProgram( vertex_shader_id, geometry_shader_id, fragment_shader_id );

		glDeleteShader( vertex_shader_id );
		if( not geometry_shader_source_path.Empty() )
			glDeleteShader( geometry_shader_id );
		glDeleteShader( fragment_shader_id );

		if( link_result )
		{
#ifdef _EDITOR
			ServiceLocator< GLLogger >::Get().SetLabel( GL_PROGRAM, program_id.Get(), GL_LABEL_PREFIX_SHADER_PROGRAM + name );
#endif // _EDITOR

			QueryVertexAttributes();

			GetUniformBookKeepingInfo();
			if( uniform_book_keeping_info.count == 0 )
				return true;

			QueryUniformData();

			ParseShaderSource_VertexLayout( *vertex_shader_source );
			ParseShaderSource_UniformAnnotations( *vertex_shader_source, ShaderType::Vertex );
			if( geometry_shader_source )
				ParseShaderSource_UniformAnnotations( *geometry_shader_source, ShaderType::Geometry );
			ParseShaderSource_UniformAnnotations( *fragment_shader_source, ShaderType::Fragment );

			QueryUniformData_BlockIndexAndOffsetForBufferMembers();
			QueryUniformBufferData( uniform_buffer_info_map_regular, Uniform::BufferCategory::Regular );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_regular );
			QueryUniformBufferData( uniform_buffer_info_map_global, Uniform::BufferCategory::Global );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_global );
			QueryUniformBufferData( uniform_buffer_info_map_intrinsic, Uniform::BufferCategory::Intrinsic );
			QueryUniformBufferData_Aggregates( uniform_buffer_info_map_intrinsic );

			CalculateTotalUniformSizes();
			EnumerateUniformBufferCategories();

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_regular )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_global )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );

			for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_intrinsic )
				UniformBlockBindingPointManager::RegisterUniformBlock( *this, uniform_buffer_name, uniform_buffer_info );
		}

		return link_result;
	}

	void Shader::Bind() const
	{
		glUseProgram( program_id.Get() );
	}

	bool Shader::RecompileFromThis( Shader& new_shader )
	{
		return new_shader.FromFile( VertexShaderSourcePath( vertex_source_path ),
									GeometryShaderSourcePath( geometry_source_path ),
									FragmentShaderSourcePath( fragment_source_path ),
									features_requested );
	}

	bool Shader::SourceFilesAreModified()
	{
		for( auto& [ source, last_write_time ] : last_write_time_map )
		{
			std::error_code error_code;
			if( const auto new_last_write_time = std::filesystem::last_write_time( source, error_code );
				not error_code )
			{
				if( new_last_write_time != last_write_time )
				{
					last_write_time = new_last_write_time;
					return true;
				}
			}
			else if( error_code == std::errc::no_such_file_or_directory )
			{
				// File is probably temporarily missing due to overwrite, ignore and check again next frame:
				continue;
			}
			else
			{
				std::cerr << "Shader::SourceFilesAreModified() (Category: " << error_code.category().name() << "): " << error_code << ".\n";
				throw std::runtime_error( "Shader::SourceFilesAreModified() (Category: " + std::string( error_code.category().name() ) + "): " + error_code.message() + ".\n" );
			}
		}

		return false;
	}

#ifdef _EDITOR
	const char* Shader::GetAnnotationFormatString( const std::uint16_t annotation_format_string_id )
	{
		return uniform_annotation_format_string_table[ annotation_format_string_id ].c_str();
	}
#endif // _EDITOR

	void Shader::SetUniform( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT					: SetUniform( uniform_info.location_or_block_index, *static_cast< const float*			>( value_pointer ) ); return;
			case GL_FLOAT_VEC2				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector2*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC3				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector3*		>( value_pointer ) ); return;
			case GL_FLOAT_VEC4				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector4*		>( value_pointer ) ); return;
			//case GL_DOUBLE					: SetUniform( uniform_info.location_or_block_index, *static_cast< const double*			>(  count_array ); return;
			case GL_INT						: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
			case GL_INT_VEC2				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector2I*		>( value_pointer ) ); return;
			case GL_INT_VEC3				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector3I*		>( value_pointer ) ); return;
			case GL_INT_VEC4				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector4I*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT			: SetUniform( uniform_info.location_or_block_index, *static_cast< const unsigned int*	>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC2		: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector2U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC3		: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector3U*		>( value_pointer ) ); return;
			case GL_UNSIGNED_INT_VEC4		: SetUniform( uniform_info.location_or_block_index, *static_cast< const Vector4U*		>( value_pointer ) ); return;
			case GL_BOOL					: SetUniform( uniform_info.location_or_block_index, *static_cast< const bool*			>( value_pointer ) ); return;
			//case GL_BOOL_VEC2				: SetUniform( uniform_info.location_or_block_index, *static_cast< const NOT DEFINED		>( value_pointer ) ); return;
			//case GL_BOOL_VEC3				: SetUniform( uniform_info.location_or_block_index, *static_cast< const NOT DEFINED		>( value_pointer ) ); return;
			//case GL_BOOL_VEC4				: SetUniform( uniform_info.location_or_block_index, *static_cast< const NOT DEFINED		>( value_pointer ) ); return;
			/* Matrices: */
			case GL_FLOAT_MAT2				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix2x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix3x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4				: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix4x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x3			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix2x3*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT2x4			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix2x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x2			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix3x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT3x4			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix3x4*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x2			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix4x2*		>( value_pointer ) ); return;
			case GL_FLOAT_MAT4x3			: SetUniform( uniform_info.location_or_block_index, *static_cast< const Matrix4x3*		>( value_pointer ) ); return;
			/* Samplers: */
			case GL_SAMPLER_1D				: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D				: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_2D_MULTISAMPLE	: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_3D				: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
			case GL_SAMPLER_CUBE			: SetUniform( uniform_info.location_or_block_index, *static_cast< const int*			>( value_pointer ) ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniform( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

	void Shader::SetUniformArray( const Uniform::Information& uniform_info, const void* value_pointer )
	{
		switch( uniform_info.type )
		{
			/* Scalars & vectors: */
			case GL_FLOAT					: SetUniformArray( uniform_info.location_or_block_index, static_cast< const float*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC2				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC3				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_VEC4				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector4*		>( value_pointer ), uniform_info.count_array ); return;
			//case GL_DOUBLE					: SetUniformArray( uniform_info.location_or_block_index, static_cast< const double*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT						: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC2				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector2I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC3				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector3I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_INT_VEC4				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector4I*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const unsigned int*	>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC2		: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector2U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC3		: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector3U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_UNSIGNED_INT_VEC4		: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Vector4U*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_BOOL					: SetUniformArray( uniform_info.location_or_block_index, static_cast< const bool*			>( value_pointer ), uniform_info.count_array ); return;
			//case GL_BOOL_VEC2				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const NOT DEFINED		>( value_pointer ), uniform_info.count_array ); return;
			//case GL_BOOL_VEC3				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const NOT DEFINED		>( value_pointer ), uniform_info.count_array ); return;
			//case GL_BOOL_VEC4				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const NOT DEFINED		>( value_pointer ), uniform_info.count_array ); return;
			/* Matrices: */
			case GL_FLOAT_MAT2				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix2x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix3x3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix4x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT2x3			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix2x3*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT2x4			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix2x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3x2			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix3x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT3x4			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix3x4*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4x2			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix4x2*		>( value_pointer ), uniform_info.count_array ); return;
			case GL_FLOAT_MAT4x3			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const Matrix4x3*		>( value_pointer ), uniform_info.count_array ); return;
			/* Samplers: */
			case GL_SAMPLER_1D				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_2D				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_2D_MULTISAMPLE	: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_3D				: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
			case GL_SAMPLER_CUBE			: SetUniformArray( uniform_info.location_or_block_index, static_cast< const int*			>( value_pointer ), uniform_info.count_array ); return;
		}

		throw std::runtime_error( "ERROR::SHADER::SetUniformArray( uniform_info, value_pointer ) called for an unknown GL type!" );
	}

/*
 *
 *	PRIVATE API:
 *
 */

	void Shader::Delete()
	{
		if( IsValid() )
		{
			glDeleteProgram( program_id.Get() );
			program_id.Reset(); // OpenGL does not reset the id to zero.
		}
	}

	std::optional< std::string > Shader::ParseShaderFromFile( const char* file_path, const ShaderType shader_type )
	{
		const std::string error_prompt( std::string( "Shader Error (parsing): " ) + ShaderTypeString( shader_type ) + " shader \"" + name + "\" could not be read successfully." );

		if( const auto source = Engine::Utility::ReadFileIntoString( file_path, error_prompt.c_str() );
			source )
			return *source;

		LogErrors( error_prompt );

		return std::nullopt;
	}

	std::vector< std::string > Shader::PreprocessShaderStage_GetIncludeFilePaths( std::string shader_source ) const
	{
		std::vector< std::string > includes;

		std::string_view shader_source_view( shader_source );

		for( auto maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "#include", R"(")", R"(")" );
			 maybe_next_token.has_value();
			 maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "#include", R"(")", R"(")" ) )
		{
			includes.emplace_back( std::string( ENGINE_SHADER_ROOT_ABSOLUTE "/" ) + std::string( *maybe_next_token ) );
		}

		return includes;
	}

	void Shader::PreprocessShaderStage_StripDefinesToBeSet( std::string& shader_source_to_modify, const std::vector< std::string >& features_to_set )
	{
		std::size_t hashtag_define_pos = 0;

		for( hashtag_define_pos = shader_source_to_modify.find( "#define", hashtag_define_pos );
			 hashtag_define_pos != std::string::npos;
			 hashtag_define_pos = shader_source_to_modify.find( "#define", hashtag_define_pos ) )
		{
			std::string_view shader_source_view( shader_source_to_modify );

			shader_source_view.remove_prefix( hashtag_define_pos );
			if( auto maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "#define", " \t", " \t\r\n" );
				maybe_next_token.has_value() &&
				/* Exclude macros; they must have parentheses at the end of the token => */ maybe_next_token->back() != ')' &&
				std::find_if( features_to_set.begin(), features_to_set.end(), [ & ]( const std::string& feature )
				{ return feature.find( *maybe_next_token ) != std::string::npos; } ) != features_to_set.end() )
			{
				if( *( shader_source_view.data() - 1 ) == '\n' )
				{
					shader_source_to_modify.erase( hashtag_define_pos,
												   shader_source_view.data() - shader_source_to_modify.data() - hashtag_define_pos );
				}
				else
				{
					shader_source_to_modify.erase( hashtag_define_pos,
												   shader_source_to_modify.find( '\n', hashtag_define_pos + 1 ) - hashtag_define_pos );
				}
			}
			else
				hashtag_define_pos++;
		}
	}

	std::unordered_map< std::string, Shader::Feature > Shader::PreProcessShaderStage_ParseFeatures( std::string shader_source )
	{
		std::unordered_map< std::string, Feature > features;

		std::string_view shader_source_view( shader_source );

		/* Parse declarations via "#pragma feature <feature_name>" syntax: */
		for( auto maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( shader_source_view, { "#pragma", "feature" }, " \t\r\n", " \t\r\n" );
			 maybe_next_token.has_value();
			 maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( shader_source_view, { "#pragma", "feature" }, " \t\r\n", " \t\r\n" ) )
		{
			features.try_emplace( std::string( *maybe_next_token ), std::nullopt, false );
		}

		/* Parse definitions via "#define <feature_name> <optional_value>" syntax: */
		for( auto maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "#define", " \t\r\n", " \t\r\n" );
			 maybe_next_token.has_value();
			 maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "#define", " \t\r\n", " \t\r\n" ) )
		{
			// TODO: The optional value may be another macro. That also needs to be replaced with the final value.
			// TODO: Maybe it is best to prohibit declaring #define features directly:
			/* If we always require #pragma feature first, we wouldn't mis-classify #defines that were not meant to be features, but constants not meant to be overridden. */

			//const auto token_pos   = shader_source_view.find_first_of( "abcdefghijklmnopqrstuvwxyz" );
			const auto token_pos   = shader_source_view.find_first_of( "0123456789." ); // Include '.' because the optional value might be ".2f" for example.
			const auto newline_pos = shader_source_view.find( '\n' );

			if( token_pos < newline_pos )
			{
				shader_source_view.remove_prefix( token_pos );
				features.try_emplace( std::string( *maybe_next_token ),
									  std::string( *Utility::String::ParseTokenAndAdvance( shader_source_view, "", " \t\r\n" ) ),
									  true );
			}
			else
				features.try_emplace( std::string( *maybe_next_token ), std::nullopt, true );
		}

		return features;
	}

	// TODO: Do not be lazy and change the line of the #defines as you please! This will make #line directives point to wrong lines.
	// Keep track of where the #define to be modified is and simply do source = source_before_define_line + define_line + source_after_define_line.

	void Shader::PreProcessShaderStage_SetFeatures( std::string& shader_source_to_modify,
													std::unordered_map< std::string, Feature >& defined_features,
													const std::vector< std::string >& features_to_set )
	{
		const auto first_new_line = shader_source_to_modify.find( "\n" );

		/* Remove all #defines THAT ARE SET by the client code from the shader, so that we can add the modified versions all in one go later.
		 * This saves us from the work of finding/replacing lines of #defines individually.
		 * We also do not remove the #define lines of macros & Features that are NOT SET by the client code. */
		PreprocessShaderStage_StripDefinesToBeSet( shader_source_to_modify, features_to_set );

		std::string define_directives_combined;
		for( const auto& feature_definition : features_to_set )
		{
			auto splitted( Utility::String::Split( feature_definition, ' ' ) );
			const std::string feature_name( std::move( splitted.front() ) );

			if( auto iterator = defined_features.find( feature_name );
				iterator != defined_features.cend() )
			{
				auto& defined_feature = iterator->second;

				defined_feature.is_set = true;

				if( splitted.size() > 1 )
				{
					const std::string feature_value( std::move( splitted[ 1 ] ) );
					defined_feature.value = feature_value;

					define_directives_combined += "#define " + feature_name + " " + feature_value + "\n";
				}
				else
					define_directives_combined += "#define " + feature_name + "\n";
			}
		}

		if( not define_directives_combined.empty() )
			shader_source_to_modify = shader_source_to_modify.substr( 0, first_new_line + 1 ) + define_directives_combined + shader_source_to_modify.substr( first_new_line + 1 );
	}

	bool Shader::PreProcessShaderStage_IncludeDirectives( const std::filesystem::path& shader_source_path,
														  std::string& shader_source_to_modify,
														  const ShaderType shader_type,
														  std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file )
	{
		char error_string[ 256 ] = { 0 };

		const std::string shader_file_name( shader_source_path.filename().string() );

		auto preprocessed_source( ShaderIncludePreprocessing::Resolve( shader_source_path, { ENGINE_SHADER_ROOT_ABSOLUTE "/" }, map_of_IDs_per_source_file, error_string ) );

		if( preprocessed_source.empty() )
		{
			const std::string error_prompt( std::string( "Shader Error (pre-compilation): " ) + ShaderTypeString( shader_type ) + " shader \"" + name + "\" could not be pre-processed for #include files.\n\t" 
											+ error_string );
			LogErrors( error_prompt );
			return false;
		}

		shader_source_to_modify = std::move( preprocessed_source );
		return true;
	}

	bool Shader::CompileShader( const char* source,
								unsigned int& shader_id,
								const ShaderType shader_type,
								std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file )
	{
		shader_id = glCreateShader( ShaderTypeID( shader_type ) );
		glShaderSource( shader_id, /* how many strings: */ 1, &source, NULL );
		glCompileShader( shader_id );

		int success = false;
		glGetShaderiv( shader_id, GL_COMPILE_STATUS, &success );
		if( !success )
		{
			LogErrors_Compilation( shader_id, shader_type, map_of_IDs_per_source_file );
			return false;
		}

		return true;
	}

	bool Shader::LinkProgram( const unsigned int vertex_shader_id, const unsigned int fragment_shader_id )
	{
		return LinkProgram( vertex_shader_id, 0, fragment_shader_id );
	}


	bool Shader::LinkProgram( const unsigned int vertex_shader_id, const unsigned int geometry_shader_id, const unsigned int fragment_shader_id )
	{
		program_id = ID( glCreateProgram() );

		glAttachShader( program_id.Get(), vertex_shader_id );
		if( geometry_shader_id > 0 )
			glAttachShader( program_id.Get(), geometry_shader_id );
		glAttachShader( program_id.Get(), fragment_shader_id );

		glLinkProgram( program_id.Get() );

		int success;
		glGetProgramiv( program_id.Get(), GL_LINK_STATUS, &success );
		if( !success )
		{
			LogErrors_Linking();
			return false;
		}

		return true;
	}

#pragma region Unnecessary Old Stuff
///* Expects: To be called after the shader whose source is passed is compiled & linked successfully. */
//	std::string Shader::ShaderSource_CommentsStripped( const std::string& shader_source )
//	{
//		/* This function is called AFTER the shader is compiled & linked. So it is known for a fact that the block comments have matching pairs of begin/end symbols. */
//
//		auto Strip = [ &shader_source ]( const std::string& source_string, const std::string& comment_begin_token, const std::string& comment_end_token,
//										 const bool do_not_erase_new_line = false )->std::string
//		{
//			std::string stripped_shader_source;
//			std::size_t current_pos = source_string.find( comment_begin_token, 0 ), last_begin_pos = 0;
//
//			while( current_pos != std::string::npos )
//			{
//				const std::size_t comment_start_pos = current_pos;
//				const std::size_t comment_end_pos = source_string.find( comment_end_token, current_pos + comment_begin_token.size() );
//
//				stripped_shader_source += source_string.substr( last_begin_pos, comment_start_pos - last_begin_pos );
//				last_begin_pos = comment_end_pos + comment_end_token.size() - ( int )do_not_erase_new_line;
//
//				current_pos = source_string.find( comment_begin_token, last_begin_pos );
//			}
//
//			/* Add the remaining part of the source string.*/
//			stripped_shader_source += source_string.substr( last_begin_pos );
//
//			return stripped_shader_source;
//		};
//
//		return Strip( Strip( shader_source, "/*", "*/" ), "//", "\n", true );
//	}
#pragma endregion

	void Shader::ParseShaderSource_UniformAnnotations( const std::string& shader_source, const ShaderType shader_type )
	{
		/* First, detect the start/end positions of all uniform blocks and cache them so we can use them in the main uniform hunting loop below. */

		struct UniformBlockSpan { std::size_t begin, end; };
		std::unordered_map< std::string, UniformBlockSpan > uniform_block_spans;

		std::string_view source_view( shader_source );

		for( auto maybe_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( source_view, { "layout", "(", "std140", ")", "uniform" }, " \t", " \t\n" );
			 maybe_token.has_value();
			 maybe_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( source_view, { "layout", "(", "std140", ")", "uniform" }, " \t", " \t\n" ) )
		{
			/* This is a valid shader semantically, so no need for error checking below. */
			const std::string uniform_block_name( *maybe_token );

			const std::size_t base = static_cast< std::size_t >( source_view.data() - shader_source.data() );

			const std::size_t begin_pos = source_view.find( '{' );
			const std::size_t end_pos   = source_view.find( '}' );

			uniform_block_spans.emplace( uniform_block_name, UniformBlockSpan{ base + begin_pos, base + end_pos } );

			source_view.remove_prefix( end_pos + 1 );
		}

		auto UniformIsInsideBlock = [ & ]( std::string& block_name )
		{
			if( auto it = std::find_if( uniform_block_spans.cbegin(), uniform_block_spans.cend(),
										[ & ]( const auto& pair )
										{
											const std::size_t source_view_pos = std::size_t( source_view.data() - shader_source.data() );
											return source_view_pos > pair.second.begin && source_view_pos < pair.second.end;
										} );
				it != uniform_block_spans.cend() )
			{
				block_name = it->first;
				return true;
			}

			return false;
		};

		auto ParseFormatStringID = [ & ]( const std::string_view arg,
										  decltype( Uniform::Information::annotation_format_string_id )& format_string_id )
		{
			if( arg.starts_with( '"' ) && arg.ends_with( '"' ) )
			{
				std::string_view format_string( arg.data() + 1, arg.size() - 2 ); // Strip double-quotes.
				if( const auto it = std::find_if( uniform_annotation_format_string_table.cbegin(), uniform_annotation_format_string_table.cend(),
												  [ & ]( const std::string& str )
												  {
													  return str.compare( format_string ) == 0;
											      } );
					it == uniform_annotation_format_string_table.cend() )
				{
					format_string_id = ( std::uint16_t )uniform_annotation_format_string_table.size();
					uniform_annotation_format_string_table.emplace_back( format_string );
				}
				else
					format_string_id = ( std::uint16_t )std::distance( uniform_annotation_format_string_table.cbegin(), it );

				return true;
			}

			return false;
		};

		/* Main uniform hunting loop; First parse the #pragma annotation line and determine the annotation type: */
		std::vector< std::string_view > splitted_annotation_line;
		source_view = std::string_view( shader_source ); // Rewind.
		while( not ( splitted_annotation_line = Utility::String::ParseAndSplitLine_SkipPrefix( source_view, "#pragma", "(,|)", "(,|)" ) ).empty() )
		{
			decltype( Uniform::Information::annotation_meta_data ) meta_data = {};
			decltype( Uniform::Information::annotation_format_string_id ) format_string_id = -1; // Sentinel value; indicates that no custom format is used.

			for( auto& token : splitted_annotation_line )
				token = Utility::String::RemoveWhitespace( token );

			const UniformAnnotation::Type annotation_type = UniformAnnotation::StringToType( splitted_annotation_line[ 0 ] );

			/* Parse meta-data for the annotation: */

			switch( annotation_type )
			{
				case UniformAnnotation::Type::Color3:
				case UniformAnnotation::Type::Color4:
				{
					BitFlags< UniformAnnotation::ColorFlags > color_flags;
					for( auto i = 1; i < splitted_annotation_line.size(); i++ )
					{
						const auto& arg = splitted_annotation_line[ i ];
						if( not ParseFormatStringID( arg, format_string_id ) )
							color_flags.Set( UniformAnnotation::StringToColorFlags( splitted_annotation_line[ i ] ) );
					}

					meta_data[ 0 ] = color_flags.ToBits();
					break;
				}
				case UniformAnnotation::Type::Driven:
					break;
				case UniformAnnotation::Type::Array:
				{
					const std::uint8_t available_dimension_count = Math::Min( 3, ( int )splitted_annotation_line.size() - 1 );
					for( auto i = 0; i < available_dimension_count; i++ )
					{
						const auto parsed_number = Utility::String::ConvertToNumber< std::uint16_t >( splitted_annotation_line[ i + 1 ] );
						std::memcpy( meta_data.data() + i * sizeof( std::uint16_t ), &parsed_number, sizeof( std::uint16_t ) );
					}
				}
					break;
				case UniformAnnotation::Type::Slider:
				{
					// Min. and max. values go to first 4 and 8 bytes respectively:
					for( auto i = 0; i < 2; i++ )
					{
						const auto parsed_number = Utility::String::ConvertToNumber< float >( splitted_annotation_line[ i + 1 ] );
						std::memcpy( meta_data.data() + i * sizeof( float ), &parsed_number, sizeof( float ) );
					}

					BitFlags< UniformAnnotation::SliderFlags > slider_flags;
					for( auto i = 3; i < splitted_annotation_line.size(); i++ )
					{
						const auto& arg = splitted_annotation_line[ i ];
						if( not ParseFormatStringID( arg, format_string_id ) )
							slider_flags.Set( UniformAnnotation::StringToSliderFlags( splitted_annotation_line[ i ] ) );
					}

					meta_data[ 8 ] = slider_flags.ToBits();

					break;
				}
				case UniformAnnotation::Type::None:
				default:
					continue;
			}

			/* Parse the uniform declaration line for the uniform name: */

			std::string uniform_name, block_name;

			const bool uniform_is_inside_block = UniformIsInsideBlock( block_name );

			if( const auto maybe_uniform_declaration_line = Utility::String::ParseNextLine( source_view );
				maybe_uniform_declaration_line.has_value() &&
				( uniform_is_inside_block || maybe_uniform_declaration_line->find( "uniform" ) != std::string::npos ) ) // Peek, not advance.
			{
				std::string_view uniform_declaration_line = *maybe_uniform_declaration_line;

				if( not uniform_is_inside_block )
					uniform_declaration_line.remove_prefix( uniform_declaration_line.find( "uniform" ) + sizeof( "uniform" ) );

				if( const auto semi_colon_pos = uniform_declaration_line.find( ';' );
					semi_colon_pos != std::string::npos )
				{
					uniform_declaration_line.remove_suffix( uniform_declaration_line.size() - semi_colon_pos );

					const auto square_bracket_pos = uniform_declaration_line.find( '[' );

					if( square_bracket_pos == std::string::npos )
						uniform_name = Utility::String::FindPreviousWord( uniform_declaration_line, uniform_declaration_line.size() );
					else
						uniform_name = std::string( Utility::String::FindPreviousWord( uniform_declaration_line, square_bracket_pos ) ) + "[0]";
				}
			}

			/* Finally register the parsed information: */

			if( not uniform_name.empty() )
			{
				// Name needs the block name prefix if inside a uniform block:
				if( uniform_is_inside_block )
					uniform_name = block_name + '.' + uniform_name;

				if( auto iterator = uniform_info_map.find( uniform_name );
					iterator != uniform_info_map.cend() )
				{
					if( annotation_type != iterator->second.annotation_type && iterator->second.annotation_type != UniformAnnotation::Type::Unassigned )
					{
						const std::string complete_error_string( std::string( "Shader Error (post-linking > parsing uniform annotations): " ) +
																 " shader \"" + name + "\" has mismatched uniform annotations across its stages.\n" );

						LogErrors( complete_error_string );
					}
					else // Finally register information:
					{
						uniform_info_map[ uniform_name ].annotation_type             = annotation_type;
						uniform_info_map[ uniform_name ].annotation_format_string_id = format_string_id;
						std::memcpy( uniform_info_map[ uniform_name ].annotation_meta_data.data(), meta_data.data(), sizeof( meta_data ) );
					}
				}
				else // Uniform does not exist? Should not be possible unless the code above has logic errors. Better safe than sorry.
				{
					/* Actually, it IS possible due to #pragma feature functionality.For example, if the uniform declaration is inside a preprocessor block that is inactive for this shader variation,
					 * then, the parsing code in this function will happily parse annotation data for it but the uniform itself will not be inside the map as the GL API will not report it.
					 * Therefore, warning logs below are commented out as it would not be correct to log this warning for the scenario explained. */

					/*const std::string complete_error_string( std::string( "Shader Error (post-linking > parsing uniform annotations): " ) + ShaderTypeString( shader_type ) +
															 " shader \"" + name + "\" has an annotation declaration that can not be matched to a valid uniform.\n" );

					LogErrors( complete_error_string );*/
				}
			}
			else
			{
				const std::string complete_error_string( std::string( "Shader Error (post-linking > parsing uniform annotations): " ) + ShaderTypeString( shader_type ) +
														 " shader \"" + name + "\" has an annotation declaration that is attached to a non-existing uniform.\n" );

				LogErrors( complete_error_string );
			}
		}
	}

	void Shader::ParseShaderSource_VertexLayout( std::string shader_source )
	{
		/* Example:
			...
			layout (location = 0) in vec3 position;
			layout (location = 1) in vec2 tex_coords;
			...
		*/
		
		std::vector< VertexAttribute > attributes;

		std::string_view shader_source_view( shader_source );

		for( auto maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( shader_source_view, { "layout", "location", "=" } );
			 maybe_next_token.has_value();
			 maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefixes( shader_source_view, { "layout", "location", "=" } ) )
		{
			const std::string_view location_sv( *maybe_next_token );
			unsigned int location;
			if( std::from_chars( location_sv.data(), location_sv.data() + location_sv.size(), location ).ec == std::errc{} )
			{
				maybe_next_token = Utility::String::ParseTokenAndAdvance_SkipPrefix( shader_source_view, "in" );
				if( maybe_next_token.has_value() )
				{
					std::string_view type_sv = *maybe_next_token;

					/* Name is not utilized at the moment => Code parsing it is disabled. */

					/*maybe_next_token = Utility::String::ParseNextTokenAndAdvance( shader_source_view, " \t", " \t\r\n" );
					if( maybe_next_token.has_value() )
					{
						std::string name_string( *maybe_next_token );*/
					std::string type_string( type_sv );

					const GLenum type( GL::Type::TypeOf( type_string.data() ) );
					/* Source attributes */
					attributes.emplace_back( GL::Type::CountOf( type ), GL::Type::ComponentTypeOf( type ), false /* => instance info does not matter. */,
											 location );
					/*}*/
				}
			}
		}

		if( not attributes.empty() )
		{
			std::sort( attributes.begin(), attributes.end(), []( const VertexAttribute& left, const VertexAttribute& right ) { return left.location < right.location; } );
			vertex_layout_source = VertexLayout( attributes );
		}
	}

	void Shader::QueryVertexAttributes()
	{
		int active_attribute_count;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_ATTRIBUTES, &active_attribute_count );

		char attribute_name[ 255 ];

		std::vector< VertexAttribute > attributes;
		attributes.reserve( active_attribute_count );
		for( auto attribute_index = 0; attribute_index < active_attribute_count; attribute_index++ )
		{
			int attribute_name_length, attribute_size;
			GLenum attribute_vector_type;
			glGetActiveAttrib( program_id.Get(), attribute_index, 255, &attribute_name_length, &attribute_size, &attribute_vector_type, attribute_name );
			const unsigned int attribute_location = glGetAttribLocation( program_id.Get(), attribute_name );

			GLint divisor_data;
			glGetIntegeri_v( GL_VERTEX_BINDING_DIVISOR, attribute_location, &divisor_data );
			const bool is_instanced = divisor_data >= 1;

			attributes.emplace_back( GL::Type::CountOf( attribute_vector_type ), GL::Type::ComponentTypeOf( attribute_vector_type ), is_instanced, attribute_location );
		}

		std::sort( attributes.begin(), attributes.end(), []( const VertexAttribute& left, const VertexAttribute& right ) { return left.location < right.location; } );
		vertex_layout_active = VertexLayout( attributes );
	}

	void Shader::GetUniformBookKeepingInfo()
	{
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORMS, &uniform_book_keeping_info.count );
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_book_keeping_info.name_max_length );
		uniform_book_keeping_info.name_holder = std::string( uniform_book_keeping_info.name_max_length, '?' );
	}

	/* Expects empty input vectors. */
	bool Shader::GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( const int active_uniform_count,
																			 std::vector< unsigned int >& block_indices, std::vector< unsigned int >& corresponding_uniform_indices ) const
	{
		corresponding_uniform_indices.resize( active_uniform_count );
		block_indices.resize( active_uniform_count );
		std::iota( corresponding_uniform_indices.begin(), corresponding_uniform_indices.end(), 0 );

		glGetActiveUniformsiv( program_id.Get(), active_uniform_count, corresponding_uniform_indices.data(), GL_UNIFORM_BLOCK_INDEX, reinterpret_cast< int* >( block_indices.data() ) );

		int block_count = 0;
		for( auto uniform_index = 0; uniform_index < active_uniform_count; uniform_index++ )
		{
			if( const auto block_index = block_indices[ uniform_index ];
				block_index != -1 )
			{
				/* Overwrite vector elements starting from the beginning. This should be safe as the uniform_index should be greater than block_count at this point.
				 * So we should be overwriting previous elements as we go through all uniform indices. */
				block_indices[ block_count ]                 = block_index;
				corresponding_uniform_indices[ block_count ] = uniform_index;
				block_count++;
			}
		}

		if( block_count )
		{
			block_indices.resize( block_count );
			corresponding_uniform_indices.resize( block_count );
			return true;
		}

		return false;
	}

	void Shader::QueryUniformData()
	{
		int offset = 0;
		for( auto uniform_index = 0; uniform_index < uniform_book_keeping_info.count; uniform_index++ )
		{
			/*
			 * glGetActiveUniform has a parameter named "size", but its actually the size of the array. So for singular types like int, float, vec2, vec3 etc. the value returned is 1.
			 */
			int array_element_count = 0, length_dontCare = 0;
			GLenum type;
			glGetActiveUniform( program_id.Get(), uniform_index, uniform_book_keeping_info.name_max_length,
								&length_dontCare, &array_element_count, &type, 
								uniform_book_keeping_info.name_holder.data() );

			const int size = GL::Type::SizeOf( type );

			const auto location = glGetUniformLocation( program_id.Get(), uniform_book_keeping_info.name_holder.c_str() );

			const bool is_buffer_member = location == -1;

			uniform_info_map[ uniform_book_keeping_info.name_holder.c_str() ] = 
			{
				.location_or_block_index = location,
				.size                    = size,
				.offset                  = is_buffer_member ? -1 : offset,
				.count_array             = array_element_count,
				.type                    = type,
				.is_buffer_member		 = is_buffer_member,
				.annotation_type         = UniformAnnotation::Type::Unassigned,
				.editor_name			 = UniformEditorName( uniform_book_keeping_info.name_holder )
			};

			offset += !is_buffer_member * size * array_element_count;
		}
	}

	void Shader::QueryUniformData_BlockIndexAndOffsetForBufferMembers()
	{
		std::vector< unsigned int > block_indices, corresponding_uniform_indices;
		if( not GetActiveUniformBlockIndicesAndCorrespondingUniformIndices( uniform_book_keeping_info.count, block_indices, corresponding_uniform_indices ) )
			return;

		std::vector< int > corresponding_offsets( corresponding_uniform_indices.size() );
		glGetActiveUniformsiv( program_id.Get(), ( int )corresponding_uniform_indices.size(), corresponding_uniform_indices.data(), GL_UNIFORM_OFFSET, corresponding_offsets.data() );

		for( int index = 0; index < corresponding_uniform_indices.size(); index++ )
		{
			const auto uniform_index = corresponding_uniform_indices[ index ];
			const auto block_index   = block_indices[ index ];
			const auto offset        = corresponding_offsets[ index ];

			int length = 0;
			glGetActiveUniformName( program_id.Get(), uniform_index, uniform_book_keeping_info.name_max_length, &length, uniform_book_keeping_info.name_holder.data() );

			const auto& uniform_name = uniform_book_keeping_info.name_holder.c_str();

			/* 'size', 'type' and 'is_buffer_member' was already initialized during query of default block uniforms. Update the remaining information for block member uniforms: */
			auto& uniform_info = uniform_info_map[ uniform_name ];

			uniform_info.location_or_block_index = ( int )block_index;
			uniform_info.offset                  = offset;
		}
	}

	void Shader::QueryUniformBufferData( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map, const Uniform::BufferCategory category_of_interest )
	{
		int active_uniform_block_count = 0;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_BLOCKS, &active_uniform_block_count );

		if( active_uniform_block_count == 0 )
			return;

		int uniform_block_name_max_length = 0;
		glGetProgramiv( program_id.Get(), GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH, &uniform_block_name_max_length );
		std::string name( uniform_block_name_max_length, '?' );

		int offset = 0;
		for( int uniform_block_index = 0; uniform_block_index < active_uniform_block_count; uniform_block_index++ )
		{
			int length = 0;
			glGetActiveUniformBlockName( program_id.Get(), uniform_block_index, uniform_block_name_max_length, &length, name.data() );
			
			int size;
			glGetActiveUniformBlockiv( program_id.Get(), uniform_block_index, GL_UNIFORM_BLOCK_DATA_SIZE,	&size );

			const auto category = Uniform::DetermineBufferCategory( name );

			if( category == category_of_interest )
			{
				auto& uniform_buffer_information = uniform_buffer_info_map[ name.c_str() ] =
				{
					.binding_point = -1, // This will be filled later via BufferManager::ConnectBufferToBlock().
					.size          = size,
					.offset        = offset,
					.category	   = category
				};

				/* Add members and set their block indices. */
				for( auto& [ uniform_name, uniform_info ] : uniform_info_map )
				{
					if( uniform_info.is_buffer_member && uniform_info.location_or_block_index == uniform_block_index )
						uniform_buffer_information.members_map.emplace( uniform_name.data(), &uniform_info );
				}

				offset += size;
			}
		}
	}

	void Shader::QueryUniformBufferData_Aggregates( std::unordered_map< std::string, Uniform::BufferInformation >& uniform_buffer_info_map )
	{
		std::vector< Uniform::Information* > members_map;

		for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
		{
			using BufferInfoPair = std::pair< const std::string, Uniform::Information* >;
			std::vector< BufferInfoPair* > uniform_buffer_info_sorted_by_offset;
			uniform_buffer_info_sorted_by_offset.reserve( uniform_buffer_info.members_map.size() );
			for( auto& pair : uniform_buffer_info.members_map )
				uniform_buffer_info_sorted_by_offset.push_back( &pair );

			std::stable_sort( uniform_buffer_info_sorted_by_offset.begin(), uniform_buffer_info_sorted_by_offset.end(),
							  []( const BufferInfoPair* left, const BufferInfoPair* right )
			{
				return left->second->offset < right->second->offset;
			} );

			for( int i = 0; i < uniform_buffer_info_sorted_by_offset.size(); i++ ) // -> Outer for loop.
			{
				const auto& buffer_info_pair = uniform_buffer_info_sorted_by_offset[ i ];
				const auto& uniform_name     = buffer_info_pair->first;
				const auto& uniform_info     = buffer_info_pair->second;

				const std::string_view uniform_name_without_buffer_name( uniform_name.cbegin() + ( uniform_name.starts_with( uniform_buffer_name )
																								   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																								   : 0 ),
																		 uniform_name.cend() );

				if( const auto bracket_pos = uniform_name_without_buffer_name.find( '[' );
					bracket_pos != std::string_view::npos )
				{
					int stride = uniform_info->size, member_count = 1;

					members_map.push_back( uniform_info );

					/* Find the other members of the array's CURRENT element: */
					bool done_processing_array_element = false;
					int j = i + 1;
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						// + 2 to include the index, which has to match for a given array element.
						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos + 2 ) ) )
						{
							stride += other_uniform_info->size;
							member_count++;
							members_map.push_back( other_uniform_info );
						}
						else
							done_processing_array_element = true;
					}

					member_count = j - i - ( done_processing_array_element ? 1 : 0 ); // -1 because j had been incremented once more before the for loop ended.
					stride       = Math::RoundToMultiple_PowerOf2( stride, sizeof( Vector4 ) ); // Std140 dictates this.

					if( done_processing_array_element )
						j--;

					done_processing_array_element = false;
					/* Now count the elements. If the array had 1 element, done_processing_array_element will cause the below loop to skip immediately. */
					for( ; j < uniform_buffer_info_sorted_by_offset.size() && not done_processing_array_element; j++ )
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						// No +2 this time; we're looking for the array name only.
						if( !other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, bracket_pos ) ) )
							done_processing_array_element = true;
					}

					const int element_count = ( j - i - ( done_processing_array_element ? 1 : 0 ) ) / member_count; // -1 because j had been incremented once more before the for loop ended.
					
					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, bracket_pos ) );
					uniform_buffer_info.members_array_map.emplace( aggregate_name,
																   Uniform::BufferMemberInformation_Array
																   { 
																		.offset        = uniform_info->offset,
																		.stride        = stride,
																		.element_count = element_count,
																		.editor_name   = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		.members_map   = members_map,
																   } );

					members_map.clear(); // Re-use the existing vector with its grown capacity.

					i += j - i - ( done_processing_array_element ? 1 : 0 ) - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1. The other -1 is the same as the ones above; for loop increments the J for one last time.
				}
				else if( const auto dot_pos = uniform_name_without_buffer_name.find( '.' );
						 dot_pos != std::string_view::npos )
				{
					int member_count = 1;
					int size = uniform_info->size;

					members_map.push_back( uniform_info );

					// Don't need to check whether the loop ended naturally or not in this case; If the loop ends naturally, then incrementing i is not important any more.

					for( int j = i + 1; j < uniform_buffer_info_sorted_by_offset.size(); j++ ) // -> Inner for loop.
					{
						const auto& other_buffer_info_pair = uniform_buffer_info_sorted_by_offset[ j ];
						const auto& other_uniform_name     = other_buffer_info_pair->first;
						const auto& other_uniform_info     = other_buffer_info_pair->second;

						const std::string_view other_uniform_name_without_buffer_name( other_uniform_name.cbegin() + ( other_uniform_name.starts_with( uniform_buffer_name )
																													   ? uniform_buffer_name.size() + 1 // +1 for the dot.
																													   : 0 ),
																					   other_uniform_name.cend() );

						if( other_uniform_name_without_buffer_name.starts_with( uniform_name_without_buffer_name.substr( 0, dot_pos ) ) )
						{
							size += other_uniform_info->size;

							members_map.push_back( other_uniform_info );
						}
						else
						{
							size = Math::RoundToMultiple_PowerOf2( size, sizeof( Vector4 ) ); // Std140 dictates this.

							member_count = j - i;
							i += member_count - 1; // Outer for loop's i++ will also increment i, that's why there is a minus 1.

							break;
						}
					}

					const auto aggregate_name( uniform_name_without_buffer_name.substr( 0, dot_pos ) );
					uniform_buffer_info.members_struct_map.emplace( aggregate_name,
																	Uniform::BufferMemberInformation_Struct
																	{
																		 .offset      = uniform_info->offset,
																		 .size        = size,
																		 .editor_name = UniformEditorName_BufferMemberAggregate( aggregate_name ),
																		 .members_map = members_map
																	} );

					members_map.clear(); // Re-use the existing vector with its grown capacity.
				}
				else
				{
					uniform_buffer_info.members_single_map.emplace( uniform_name_without_buffer_name, uniform_info );
				}
			}
		}
	}

	void Shader::CalculateTotalUniformSizes()
	{
		uniform_book_keeping_info.default_block_size = 0;

		/* Sum of default block (i.e., not in any explicit Uniform Buffer) uniforms: */
		for( const auto& [ uniform_name, uniform_info ] : uniform_info_map )
			if( not uniform_info.is_buffer_member ) // Skip buffer members, as their layout (std140) means their total buffer size is calculated differently.
				uniform_book_keeping_info.default_block_size += uniform_info.size * uniform_info.count_array;

		/* Now add buffer block sizes (calculated before): */
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_regular )
			uniform_book_keeping_info.regular_total_size = uniform_buffer_info.size;
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_global )
			uniform_book_keeping_info.global_total_size = uniform_buffer_info.size;
		for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map_intrinsic )
			uniform_book_keeping_info.intrinsic_total_size = uniform_buffer_info.size;

		uniform_book_keeping_info.total_size = uniform_book_keeping_info.default_block_size + uniform_book_keeping_info.TotalSize_Blocks();
	}

	void Shader::EnumerateUniformBufferCategories()
	{
		uniform_book_keeping_info.regular_block_count   = ( int )uniform_buffer_info_map_regular.size();
		uniform_book_keeping_info.global_block_count    = ( int )uniform_buffer_info_map_global.size();
		uniform_book_keeping_info.intrinsic_block_count = ( int )uniform_buffer_info_map_intrinsic.size();
	}

	const Uniform::Information*  Shader::GetUniformInformation( const std::string& uniform_name )
	{
#ifdef _EDITOR
		if( const auto iterator = uniform_info_map.find( uniform_name );
			iterator != uniform_info_map.cend() )
		{
			return &iterator->second;
		}
		else
		{
			ServiceLocator< GLLogger >::Get().Warning( R"(Shader::GetUniformInformation(): Shader ")" + name + R"(" does not define the uniform ")" + std::string( uniform_name ) + R"(" (may be optimized away).)" );
			return nullptr;
		}
#else // STANDALONE:
		return uniform_info_map[ uniform_name ];
#endif // _EDITOR
	}

	void Shader::LogErrors( const std::string& error_string ) const
	{
		std::cerr << error_string;
#if defined( _WIN32 ) && defined( _DEBUG )
		if( IsDebuggerPresent() )
			OutputDebugStringA( ( "\n" + error_string + "\n" ).c_str() );
#endif // _WIN32 && _DEBUG

		ServiceLocator< GLLogger >::Get().Error( error_string );
	}

	void Shader::LogErrors_Compilation( const int shader_id,
										const ShaderType shader_type,
										std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file ) const
	{
		char info_log[ 512 ];
		int info_log_length;
		glGetShaderInfoLog( shader_id, 512, &info_log_length, info_log );

		const std::string complete_error_string( std::string( "Shader Error (compilation): " ) + ShaderTypeString( shader_type ) + " shader \"" + name + "\"" +
												 FormatErrorLog( info_log, info_log_length, map_of_IDs_per_source_file ) );
		LogErrors( complete_error_string );
	}

	void Shader::LogErrors_Linking() const
	{
		char info_log[ 512 ];
		glGetProgramInfoLog( program_id.Get(), 512, NULL, info_log );

		const std::string complete_error_string( "Shader Error (linking): Shader \"" + name + "\"" + FormatErrorLog( info_log ) );
		LogErrors( complete_error_string );
	}

	std::string Shader::FormatErrorLog( const char* log ) const
	{
		std::string error_log_string( log );
		if( error_log_string.back() == '\n' )
			error_log_string.pop_back();

		Utility::String::Replace( error_log_string, "\n", "\n    " );
		return "\n    " + error_log_string + "\n";
	}

	/* Replaces file IDs with actual file paths. */
	std::string Shader::FormatErrorLog( const char* log,
										const int log_length,
										std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file ) const
	{
		const auto& graphics_device_vendor = ServiceLocator< Graphics::DeviceInfo >::Get().vendor;

		std::string info_log_str( "\n" );
		info_log_str.reserve( log_length );

		std::string_view info_log_view( log );
		auto maybe_next_log_line = Utility::String::ParseNextLineAndAdvance( info_log_view );

		while( maybe_next_log_line.has_value() )
		{
			const std::string_view line = *maybe_next_log_line;

			/*
			 * NVIDIA
			 * 
			 * Expected format: <id>(<line>) error ...
			 * Example: 0(123) error ...
			 */
			if( graphics_device_vendor == Graphics::DeviceInfo::Vendor::Nvidia )
			{
				const std::size_t paren_pos = line.find( "(" );

				char file_ID_string[ 8 ];
				strncpy_s( file_ID_string, line.data(), paren_pos );

				const std::int16_t file_ID = std::atoi( file_ID_string );

				info_log_str += '\t' + map_of_IDs_per_source_file[ file_ID ].string() + " -> line " + std::string( line.substr( paren_pos ) ) + ".\n";

			}
			/*
			 * AMD
			 *
			 * Expected format: <id>:<line>: error ...
			 * Example: ERROR: 0:123: ...
			 */
			else if( graphics_device_vendor == Graphics::DeviceInfo::Vendor::Amd )
			{
				// Expected: ERROR: <id>:<line>: ...
				const std::size_t first_colon = line.find( ':' );
				if( first_colon != std::string_view::npos )
				{
					const std::size_t second_colon = line.find( ':', first_colon + 1 );

					if( second_colon != std::string_view::npos )
					{
						const std::int16_t file_id = static_cast< std::int16_t >(
							std::atoi( std::string( line.substr( first_colon + 1,
																 second_colon - first_colon - 1 ) ).c_str() ) );

						const std::size_t third_colon = line.find( ':', second_colon + 1 );

						const int line_number = std::atoi( std::string( line.substr( second_colon + 1,
																					 third_colon - second_colon - 1 ) ).c_str() );

						info_log_str += '\t' + map_of_IDs_per_source_file[ file_id ].string() + " -> line " + std::to_string( line_number ) + ": " +
							std::string( line.substr( third_colon + 1 ) ) + '\n';
					}
				}
			}

			/*
			 * Intel/Mesa
			 * 
			 * Expected format: <id>:<line>(<col>): ...
			 */
			else if( graphics_device_vendor == Graphics::DeviceInfo::Vendor::Intel ||
					 graphics_device_vendor == Graphics::DeviceInfo::Vendor::Mesa )
			{
				const std::size_t first_colon = line.find( ':' );
				if( first_colon != std::string_view::npos )
				{
					const std::int16_t file_id =
						static_cast< std::int16_t >(
							std::atoi(
								std::string( line.substr( 0, first_colon ) ).c_str() ) );

					const std::size_t paren_pos = line.find( '(' );
					if( paren_pos != std::string_view::npos )
					{
						const int line_number = std::atoi(
							std::string( line.substr( first_colon + 1,
													  paren_pos - first_colon - 1 ) ).c_str() );

						info_log_str += '\t' + map_of_IDs_per_source_file[ file_id ].string() + " -> line " + std::to_string( line_number ) + ": " + std::string( line ) +'\n';
					}
				}
			}
			/*
			 * Unknown:
			 */
			else
			{
				info_log_str += '\t';
				info_log_str += line;
				info_log_str += '\n';
			}

			maybe_next_log_line = Utility::String::ParseNextLineAndAdvance( info_log_view );
		}

		return info_log_str;
	}

	std::string Shader::UniformEditorName( const std::string_view original_name )
	{
		const auto null_terminator_pos = original_name.find( '\0' );
		std::string editor_name( original_name.cbegin(), null_terminator_pos != std::string::npos
																? original_name.cbegin() + null_terminator_pos + 1
																: original_name.cend() );

		/* First get rid of parent struct/block names: */
		if( const auto last_dot_pos = editor_name.find_last_of( '.' );
			last_dot_pos != std::string::npos )
			editor_name = editor_name.substr( last_dot_pos + 1 );

		if( editor_name.compare( 0, 7, "_GLOBAL", 7 ) == 0 )
			editor_name.erase( 0, 7 );
		else if( editor_name.compare( 0, 8, "_REGULAR", 8 ) == 0 )
			editor_name.erase( 0, 8 );
		else if( editor_name.compare( 0, 10, "_INTRINSIC", 10 ) == 0 )
			editor_name.erase( 0, 10 );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( editor_name.compare( 0, 8, "UNIFORM ", 8 ) == 0 || editor_name.compare( 0, 8, "uniform ", 8 ) == 0 )
			editor_name.erase( 0, 8 );

		if( editor_name.compare( 0, 4, "TEX ", 4 ) == 0 || editor_name.compare( 0, 4, "tex ", 4 ) == 0 )
			editor_name.erase( 0, 4 );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		/* Get rid of the 0 inside [0] for arrays: */
		if( const auto bracket_zero_pos = editor_name.find( "[0]" );
			bracket_zero_pos != std::string::npos )
			editor_name.erase( bracket_zero_pos + 1, 1 );

		return editor_name;
	}

	std::string Shader::UniformEditorName_BufferMemberAggregate( const std::string_view aggregate_name )
	{
		std::string editor_name( aggregate_name );

		std::replace( editor_name.begin(), editor_name.end(), '_', ' ' );

		if( std::isalpha( editor_name[ 0 ] ) )
			editor_name[ 0 ] = std::toupper( editor_name[ 0 ] );
		else if( editor_name.starts_with( ' ' ) )
			editor_name = editor_name.erase( 0, 1 );

		for( auto index = 1; index < editor_name.size(); index++ )
			if( editor_name[ index - 1 ] == ' ' )
				editor_name[ index ] = std::toupper( editor_name[ index ] );

		return editor_name;
	}
}
