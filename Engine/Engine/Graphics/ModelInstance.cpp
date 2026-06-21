// Engine Includes.
#include "ModelInstance.h"
#include "Graphics/BuiltinMaterials.h"
#include "Graphics/BuiltinTextures.h"

namespace Kakadu
{
	ModelInstance::ModelInstance()
		:
		model( nullptr ),
		shader( nullptr ),
		shader_shadow_receiving( nullptr )
	{}

	// TODO: Pass name here and use that name as the Renderable name. Currently, Material name becomes the Renderable name.
	ModelInstance::ModelInstance( const Model* model, 
								  RHI::Shader* const shader,
								  RHI::Shader* const shader_shadow_receiving,
								  const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
								  Material* material,
								  const bool receives_shadows,
								  const bool casts_shadows,
								  const Vector4 texture_scale_and_offset )
		:
		model( model ),
		shader( shader ),
		shader_shadow_receiving( shader_shadow_receiving ),
		texture_scale_and_offset( texture_scale_and_offset )
	{
		ASSERT_DEBUG_ONLY( model != nullptr );
		ASSERT_EDITOR_ONLY( shader && "Shader of the model instance is nullptr." );
		ASSERT_EDITOR_ONLY( shader_shadow_receiving && "Shader (shadow receiving) of the model instance is nullptr." );

		SetMaterialData( receives_shadows ? shader_shadow_receiving : shader, texture_scale_and_offset );

		/* Initialize Transforms, Materials & Renderables of Nodes: */

		const auto  mesh_instance_count = model->MeshInstanceCount();
		const auto& meshes              = model->Meshes();
		const auto& nodes               = model->Nodes();

		node_world_matrix_array.resize( mesh_instance_count );
		node_renderable_array.resize( mesh_instance_count );

		/* Apply scene-graph transformations: */

		i32 mesh_index = 0;
		std::function< void( const std::size_t, const Matrix4x4& ) > ProcessNode = [ & ]( const std::size_t node_index, const Matrix4x4& parent_transform )
		{
			const auto& node = nodes[ node_index ];

			auto transform_so_far = node.transform_local * parent_transform;

			for( auto& child_index : node.children )
				ProcessNode( child_index, transform_so_far );

			if( node.mesh_group )
			{
				for( auto& mesh_info : node.mesh_group->mesh_infos )
				{
					node_world_matrix_array[ mesh_index ] = transform_so_far;

					Material* const mat = material ? material
					                    : ( mesh_info.material_info_index >= 0
											? &node_material_array[ mesh_info.material_info_index ]
											: receives_shadows ? BuiltinMaterials::Get( "Default (Shadowed)" ) : BuiltinMaterials::Get( "Default" ) );

					node_renderable_array[ mesh_index ] = Renderable( &mesh_info.mesh, mat, nullptr, receives_shadows, casts_shadows,
					                                                  mat && mat->HasUniform( "uniform_transform_world" )
					                                                    ? &node_world_matrix_array[ mesh_index ]
					                                                    : nullptr );

					mesh_index++;
				}
			}
		};

		for( auto top_level_node_index : model->TopLevelNodeIndices() )
			ProcessNode( top_level_node_index, Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) * Matrix::Translation( translation ) );
	}

	ModelInstance::~ModelInstance()
	{}

	void ModelInstance::SetTransformation( const Matrix4x4& transformation )
	{
		i32 mesh_index = 0;
		std::function< void( const std::size_t, const Matrix4x4& ) > ProcessNode = [ & ]( const std::size_t node_index, const Matrix4x4& parent_transform )
		{
			const auto& node = model->Nodes()[ node_index ];

			auto transform_so_far = node.transform_local * parent_transform;

			for( auto& child_index : node.children )
				ProcessNode( child_index, transform_so_far );

			if( node.mesh_group )
			{
				for( auto& mesh_info : node.mesh_group->mesh_infos )
				{
					node_world_matrix_array[ mesh_index ] = transform_so_far;

					mesh_index++;
				}
			}
		};

		for( auto top_level_node_index : model->TopLevelNodeIndices() )
			ProcessNode( top_level_node_index, transformation );
	}

	internal_function void PopulateMaterial( Material& material, const Model::MaterialInfo& material_info,
	                                         MaterialData::BlinnPhongMaterialData& blinn_phong_data,
	                                         const Vector4& texture_scale_and_offset )
	{
		if( material_info.texture_albedo )
		{
			blinn_phong_data =
			{
				.color_diffuse       = {},
				.has_texture_diffuse = 1,
				.shininess           = 32.0f
			};

			material.SetTexture( "uniform_tex_diffuse", material_info.texture_albedo );
		}
		else if( material_info.color_albedo )
		{
			blinn_phong_data =
			{
				.color_diffuse       = *material_info.color_albedo,
				.has_texture_diffuse = 0,
				.shininess           = 32.0f
			};
		}

		material.SetTexture( "uniform_tex_normal", material_info.texture_normal ? material_info.texture_normal : BuiltinTextures::Get( "Normal Map" ) );
		material.SetTexture( "uniform_tex_specular", BuiltinTextures::Get( "White" ) );
		material.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );
	}

	void ModelInstance::SetMaterialData( RHI::Shader* const shader, const Vector4 texture_scale_and_offset )
	{
		ASSERT_EDITOR_ONLY( shader && "Shader passed to ModelInstance::SetMaterialData is nullptr." );

		const auto& material_infos = model->MaterialInfos();

		node_material_array.resize( model->MaterialInfoCount() );
		blinn_phong_material_data_array.resize( model->MaterialInfoCount() );

		for( i32 i = 0; i < ( i32 )material_infos.size(); i++ )
		{
			const auto& material_info = material_infos[ i ];
			auto& material = node_material_array[ i ] = Material( material_info.name, shader );
			PopulateMaterial( material, material_info, blinn_phong_material_data_array[ i ], texture_scale_and_offset );
		}

		for( i32 i = 0; i < ( i32 )blinn_phong_material_data_array.size(); i++ )
			node_material_array[ i ].Set( "BlinnPhongMaterialData", blinn_phong_material_data_array[ i ] );
	}

	void ModelInstance::ToggleShadowReceivingStatus( const bool receive_shadows )
	{
		if( receive_shadows == node_renderable_array.front().is_receiving_shadows )
			return;

		ASSERT_EDITOR_ONLY( shader && "Shader of the model instance is nullptr." );
		ASSERT_EDITOR_ONLY( shader_shadow_receiving && "Shader (shadow receiving) of the model instance is nullptr." );

		const auto& material_infos = model->MaterialInfos();
		const auto  shader_to_set  = receive_shadows ? shader_shadow_receiving : shader;

		for( i32 i = 0; i < ( i32 )material_infos.size(); i++ )
		{
			auto& material = node_material_array[ i ];
			material.SetShader( shader_to_set );
			PopulateMaterial( material, material_infos[ i ], blinn_phong_material_data_array[ i ], texture_scale_and_offset );
		}

		for( i32 i = 0; i < ( i32 )blinn_phong_material_data_array.size(); i++ )
			node_material_array[ i ].Set( "BlinnPhongMaterialData", blinn_phong_material_data_array[ i ] );

		for( auto& renderable : node_renderable_array )
			renderable.is_receiving_shadows = receive_shadows;
	}

	void ModelInstance::ToggleShadowCastingStatus( const bool cast_shadows )
	{
		for( auto& renderable : node_renderable_array )
			renderable.is_receiving_shadows = cast_shadows;
	}
}