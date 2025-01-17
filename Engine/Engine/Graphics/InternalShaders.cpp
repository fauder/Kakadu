// Engine Includes.
#include "InternalShaders.h"
#include "Renderer.h"
#include "Core/Utility.hpp"
#include "Asset/Shader/InternalShaderDirectoryPath.h"

#define FullShaderPath( file_path ) Utility::String::ConstexprConcatenate( Engine::SHADER_SOURCE_DIRECTORY_WITH_SEPARATOR_AS_ARRAY,\
																		   Utility::String::StringViewToArray< std::string_view( file_path ).size() >( std::string_view( file_path ) ) )

namespace Engine
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Shader > InternalShaders::SHADER_MAP;

	Shader* InternalShaders::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( SHADER_MAP.contains( name ) && ( "Built-in shader with the name \"" + name + "\" was not found!" ).c_str() );

		return &( SHADER_MAP.find( name )->second );
	}

	void InternalShaders::Initialize( Renderer& renderer )
	{
		using namespace Literals;

		SHADER_MAP.try_emplace( "Skybox",
								"Skybox",
								FullShaderPath( "Skybox.vert"_vert ),
								FullShaderPath( "Skybox.frag"_frag ) );
		SHADER_MAP.try_emplace( "Blinn-Phong",
								"Blinn-Phong",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ) );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed)",
								"Blinn-Phong (Shadowed)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Instanced)",
								"Blinn-Phong (Instanced)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection)",
								"Blinn-Phong (Skybox Reflection)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Instanced)",
								"Blinn-Phong (Shadowed | Instanced)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Parallax)",
								"Blinn-Phong (Shadowed | Parallax)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "PARALLAX_MAPPING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Parallax | Instanced)",
								"Blinn-Phong (Shadowed | Parallax | Instanced)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "PARALLAX_MAPPING_ENABLED",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection | Instanced)",
								"Blinn-Phong (Skybox Reflection | Instanced)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection | Shadowed | Instanced)",
								"Blinn-Phong (Skybox Reflection | Shadowed | Instanced)",
								FullShaderPath( "Blinn-Phong.vert"_vert ),
								FullShaderPath( "Blinn-Phong.frag"_frag ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING",
												  "SHADOWS_ENABLED", 
												  "SOFT_SHADOWS",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Color",
								"Color",
								FullShaderPath( "Color.vert"_vert ),
								FullShaderPath( "Color.frag"_frag ) );
		SHADER_MAP.try_emplace( "Color (Instanced)",
								"Color (Instanced)",
								FullShaderPath( "Color.vert"_vert ),
								FullShaderPath( "Color.frag"_frag ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Textured",
								"Textured",
								FullShaderPath( "Textured.vert"_vert ),
								FullShaderPath( "Textured.frag"_frag ) );
		SHADER_MAP.try_emplace( "Textured (Discard Transparent)",
								"Textured (Discard Transparent)",
								FullShaderPath( "Textured.vert"_vert ),
								FullShaderPath( "Textured.frag"_frag ),
								Shader::Features{ "DISCARD_TRANSPARENT_FRAGMENTS" } );
		SHADER_MAP.try_emplace( "Outline",
								"Outline",
								FullShaderPath( "Outline.vert"_vert ),
								FullShaderPath( "Color.frag"_frag ) );
		SHADER_MAP.try_emplace( "Texture Blit",
								"Texture Blit",
								FullShaderPath( "PassThrough_UVs.vert"_vert ),
								FullShaderPath( "Textured.frag"_frag ) );
		SHADER_MAP.try_emplace( "Fullscreen Blit",
								"Fullscreen Blit",
								FullShaderPath( "PassThrough.vert"_vert ),
								FullShaderPath( "FullScreenBlit.frag"_frag ) );
		SHADER_MAP.try_emplace( "Fullscreen Blit Resolve",
								"Fullscreen Blit Resolve",
								FullShaderPath( "PassThrough.vert"_vert ),
								FullShaderPath( "FullScreenBlit_Resolve.frag"_frag ) );
		SHADER_MAP.try_emplace( "Post-process Grayscale",
								"Post-process Grayscale",
								FullShaderPath( "PassThrough.vert"_vert ),
								FullShaderPath( "Grayscale.frag"_frag ) );
		SHADER_MAP.try_emplace( "Post-process Generic",
								"Post-process Generic",
								FullShaderPath( "PassThrough.vert"_vert ),
								FullShaderPath( "GenericPostprocess.frag"_frag ) );
		SHADER_MAP.try_emplace( "Normal Visualization",
								"Normal Visualization",
								FullShaderPath( "VisualizeNormals.vert"_vert ),
								FullShaderPath( "VisualizeNormals.geom"_geom ),
								FullShaderPath( "VisualizeNormals.frag"_frag ) );
		SHADER_MAP.try_emplace( "Shadow-map Write",
								"Shadow-map Write",
								FullShaderPath( "PassThrough_Transform.vert"_vert ),
								FullShaderPath( "Empty.frag"_frag ) );
		SHADER_MAP.try_emplace( "Shadow-map Write (Instanced)",
								"Shadow-map Write (Instanced)",
								FullShaderPath( "PassThrough_Transform.vert"_vert ),
								FullShaderPath( "Empty.frag"_frag ),
								Shader::Features{ "INSTANCING_ENABLED" } );

		/* Register all built-in shaders: */
		for( auto& [ shader_name, shader ] : SHADER_MAP )
			renderer.RegisterShader( shader );
	}
}
