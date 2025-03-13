// Engine Includes.
#include "BuiltinShaders.h"
#include "Renderer.h"
#include "Core/Utility.hpp"
#include "Asset/AssetDirectoryPath.h"

#define FullVertexShaderPath( file_path )	ENGINE_ASSET_PATH( "Shader/" file_path##_vert )
#define FullGeometryShaderPath( file_path ) ENGINE_ASSET_PATH( "Shader/" file_path##_geom )
#define FullFragmentShaderPath( file_path ) ENGINE_ASSET_PATH( "Shader/" file_path##_frag )

namespace Engine
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Shader > BuiltinShaders::SHADER_MAP;

	Shader* BuiltinShaders::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( SHADER_MAP.contains( name ) && ( "Built-in shader with the name \"" + name + "\" was not found!" ).c_str() );

		return &( SHADER_MAP.find( name )->second );
	}

	void BuiltinShaders::Initialize( Renderer& renderer )
	{
		using namespace Literals;

		SHADER_MAP.try_emplace( "Skybox",
								"Skybox",
								FullVertexShaderPath( "Skybox.vert" ),
								FullFragmentShaderPath( "Skybox.frag" ) );
		SHADER_MAP.try_emplace( "Blinn-Phong",
								"Blinn-Phong",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ) );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed)",
								"Blinn-Phong (Shadowed)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Instanced)",
								"Blinn-Phong (Instanced)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection)",
								"Blinn-Phong (Skybox Reflection)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Instanced)",
								"Blinn-Phong (Shadowed | Instanced)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Parallax)",
								"Blinn-Phong (Shadowed | Parallax)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "PARALLAX_MAPPING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Shadowed | Parallax | Instanced)",
								"Blinn-Phong (Shadowed | Parallax | Instanced)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SHADOWS_ENABLED",
												  "SOFT_SHADOWS",
												  "PARALLAX_MAPPING_ENABLED",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection | Instanced)",
								"Blinn-Phong (Skybox Reflection | Instanced)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Blinn-Phong (Skybox Reflection | Shadowed | Instanced)",
								"Blinn-Phong (Skybox Reflection | Shadowed | Instanced)",
								FullVertexShaderPath( "Blinn-Phong.vert" ),
								FullFragmentShaderPath( "Blinn-Phong.frag" ),
								Shader::Features{ "SKYBOX_ENVIRONMENT_MAPPING",
												  "SHADOWS_ENABLED", 
												  "SOFT_SHADOWS",
												  "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Color",
								"Color",
								FullVertexShaderPath( "Color.vert" ),
								FullFragmentShaderPath( "Color.frag" ) );
		SHADER_MAP.try_emplace( "Color (Instanced)",
								"Color (Instanced)",
								FullVertexShaderPath( "Color.vert" ),
								FullFragmentShaderPath( "Color.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
#ifdef _EDITOR
		SHADER_MAP.try_emplace( "Debug TBN As Colors",
								"Debug TBN As Colors",
								FullVertexShaderPath( "DebugTBN_AsColors.vert" ),
								FullFragmentShaderPath( "DebugTBN_AsColors.frag" ) );
		SHADER_MAP.try_emplace( "Debug TBN As Colors (Instanced)",
								"Debug TBN As Colors (Instanced)",
								FullVertexShaderPath( "DebugTBN_AsColors.vert" ),
								FullFragmentShaderPath( "DebugTBN_AsColors.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Debug TBN As Vectors",
								"Debug TBN As Vectors",
								FullVertexShaderPath( "DebugTBN_AsVectors.vert" ),
								FullGeometryShaderPath( "DebugTBN_AsVectors.geom" ),
								FullFragmentShaderPath( "DebugTBN_AsVectors.frag" ) );
		SHADER_MAP.try_emplace( "Debug TBN As Vectors (Instanced)",
								"Debug TBN As Vectors (Instanced)",
								FullVertexShaderPath( "DebugTBN_AsVectors.vert" ),
								FullGeometryShaderPath( "DebugTBN_AsVectors.geom" ),
								FullFragmentShaderPath( "DebugTBN_AsVectors.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Debug UVs As Colors",
								"Debug UVs As Colors",
								FullVertexShaderPath( "DebugUVs_AsColors.vert" ),
								FullFragmentShaderPath( "DebugUVs_AsColors.frag" ) );
		SHADER_MAP.try_emplace( "Debug UVs As Colors (Instanced)",
								"Debug UVs As Colors (Instanced)",
								FullVertexShaderPath( "DebugUVs_AsColors.vert" ),
								FullFragmentShaderPath( "DebugUVs_AsColors.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
#endif // _EDITOR
		SHADER_MAP.try_emplace( "Textured",
								"Textured",
								FullVertexShaderPath( "Textured.vert" ),
								FullFragmentShaderPath( "Textured.frag" ) );
		SHADER_MAP.try_emplace( "Textured (Discard Transparent)",
								"Textured (Discard Transparent)",
								FullVertexShaderPath( "Textured.vert" ),
								FullFragmentShaderPath( "Textured.frag" ),
								Shader::Features{ "DISCARD_TRANSPARENT_FRAGMENTS" } );
		SHADER_MAP.try_emplace( "Outline",
								"Outline",
								FullVertexShaderPath( "Outline.vert" ),
								FullFragmentShaderPath( "Color.frag" ) );
		SHADER_MAP.try_emplace( "Texture Blit",
								"Texture Blit",
								FullVertexShaderPath( "PassThrough_UVs.vert" ),
								FullFragmentShaderPath( "Textured.frag" ) );
		SHADER_MAP.try_emplace( "Fullscreen Blit",
								"Fullscreen Blit",
								FullVertexShaderPath( "PassThrough.vert" ),
								FullFragmentShaderPath( "FullScreenBlit.frag" ) );
		SHADER_MAP.try_emplace( "MSAA Resolve",
								"MSAA Resolve",
								FullVertexShaderPath( "PassThrough.vert" ),
								FullFragmentShaderPath( "MSAA_Resolve.frag" ) );
		SHADER_MAP.try_emplace( "Tone Mapping",
								"Tone Mapping",
								FullVertexShaderPath( "PassThrough.vert" ),
								FullFragmentShaderPath( "Tonemapping.frag" ) );
		SHADER_MAP.try_emplace( "Post-process Grayscale",
								"Post-process Grayscale",
								FullVertexShaderPath( "PassThrough.vert" ),
								FullFragmentShaderPath( "Grayscale.frag" ) );
		SHADER_MAP.try_emplace( "Post-process Generic",
								"Post-process Generic",
								FullVertexShaderPath( "PassThrough.vert" ),
								FullFragmentShaderPath( "GenericPostprocess.frag" ) );
		SHADER_MAP.try_emplace( "Shadow-map Write",
								"Shadow-map Write",
								FullVertexShaderPath( "PassThrough_Transform.vert" ),
								FullFragmentShaderPath( "Empty.frag" ) );
		SHADER_MAP.try_emplace( "Shadow-map Write (Instanced)",
								"Shadow-map Write (Instanced)",
								FullVertexShaderPath( "PassThrough_Transform.vert" ),
								FullFragmentShaderPath( "Empty.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Wireframe",
								"Wireframe",
								FullVertexShaderPath( "Wireframe.vert" ),
								FullGeometryShaderPath( "Wireframe.geom" ),
								FullFragmentShaderPath( "Wireframe.frag" ) );
		SHADER_MAP.try_emplace( "Wireframe (Instanced)",
								"Wireframe (Instanced)",
								FullVertexShaderPath( "Wireframe.vert" ),
								FullGeometryShaderPath( "Wireframe.geom" ),
								FullFragmentShaderPath( "Wireframe.frag" ),
								Shader::Features{ "INSTANCING_ENABLED" } );
		SHADER_MAP.try_emplace( "Shaded Wireframe",
								"Shaded Wireframe",
								FullVertexShaderPath( "Wireframe.vert" ),
								FullGeometryShaderPath( "Wireframe.geom" ),
								FullFragmentShaderPath( "Wireframe.frag" ),
								Shader::Features{ "OFFSET_DEPTH" } );
		SHADER_MAP.try_emplace( "Shaded Wireframe (Instanced)",
								"Shaded Wireframe (Instanced)",
								FullVertexShaderPath( "Wireframe.vert" ),
								FullGeometryShaderPath( "Wireframe.geom" ),
								FullFragmentShaderPath( "Wireframe.frag" ),
								Shader::Features{ "INSTANCING_ENABLED", "OFFSET_DEPTH" } );

		/* Register all built-in shaders: */
		for( auto& [ shader_name, shader ] : SHADER_MAP )
			renderer.RegisterShader( shader );
	}
}
