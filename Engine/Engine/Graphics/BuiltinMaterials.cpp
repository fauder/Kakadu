// Engine Includes.
#include "BuiltinMaterials.h"
#include "BuiltinShaders.h"
#include "BuiltinTextures.h"
#include "Renderer.h"
#include "MaterialData/MaterialData.h"
#include "Asset/Paths.h"
#include "Core/AssetDatabase.hpp"
#include "Core/Utility.hpp"

namespace Kakadu
{
	/* Static member variable definitions: */
	std::unordered_map< std::string, Material > BuiltinMaterials::MATERIAL_MAP;

	Material* BuiltinMaterials::Get( const std::string& name )
	{
		// Just to get a better error message.
		ASSERT_DEBUG_ONLY( MATERIAL_MAP.contains( name ) && ( "Built-in material with the name \"" + name + "\" was not found!" ).c_str() );

		return &( MATERIAL_MAP.find( name )->second );
	}

	void BuiltinMaterials::Initialize()
	{
		constexpr Vector4 texture_scale_and_offset = Vector4{ 1.0f, 1.0f, 0.0f, 0.0f };

		using namespace Math::Literals;

		constexpr MaterialData::BlinnPhongMaterialData blinn_phong_material_data =
		{
			.color_diffuse       = Color3::Gray( ( float )50_percent ),
			.has_texture_diffuse = 0,
			.shininess           = 32.0f
		};

		/* Default Material: */
		Material& default_mat = MATERIAL_MAP.try_emplace( "Default", "Default", BuiltinShaders::Get( "Blinn-Phong" ) ).first->second;

		default_mat.SetTexture( "uniform_tex_normal", BuiltinTextures::Get( "Normal Map" ) );
		default_mat.SetTexture( "uniform_tex_specular", BuiltinTextures::Get( "Black" ) );
		default_mat.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );
		default_mat.Set( "BlinnPhongMaterialData", blinn_phong_material_data );

		/* Default Shadowed Material: */
		Material& default_shadowed_mat =
			MATERIAL_MAP.try_emplace( "Default (Shadowed)", "Default (Shadowed)", BuiltinShaders::Get( "Blinn-Phong (Shadowed)" ) ).first->second;

		default_shadowed_mat.SetTexture( "uniform_tex_normal", BuiltinTextures::Get( "Normal Map" ) );
		default_shadowed_mat.SetTexture( "uniform_tex_specular", BuiltinTextures::Get( "Black" ) );
		default_shadowed_mat.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );
		default_shadowed_mat.Set( "BlinnPhongMaterialData", blinn_phong_material_data );
	}
}
