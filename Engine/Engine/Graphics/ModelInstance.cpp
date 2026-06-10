// Engine Includes.
#include "ModelInstance.h"
#include "Graphics/BuiltinTextures.h"

namespace Kakadu
{
	ModelInstance::ModelInstance()
		:
		model( nullptr ),
		shader( nullptr ),
		shader_shadow_receiving( nullptr )
	{}

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

		node_transform_array.resize( mesh_instance_count );
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
					node_transform_array[ mesh_index ].SetFromSRTMatrix( transform_so_far );
					node_renderable_array[ mesh_index ] = Renderable( &mesh_info.mesh, ( material ? material : &node_material_array[ mesh_index ] ),
																	  node_material_array[ mesh_index ].HasUniform( "uniform_transform_world" )
																		? &node_transform_array[ mesh_index ]
																		: nullptr,
																	  receives_shadows, casts_shadows );

					mesh_index++;
				}
			}
		};

		for( auto top_level_node_index : model->TopLevelNodeIndices() )
			ProcessNode( top_level_node_index, Matrix::Scaling( scale ) * Math::QuaternionToMatrix( rotation ) * Matrix::Translation( translation ) );
	}

	ModelInstance::~ModelInstance()
	{}

	void ModelInstance::SetMaterialData( RHI::Shader* const shader, const Vector4 texture_scale_and_offset )
	{
		i32 material_index = 0;

		ASSERT_EDITOR_ONLY( shader && "Shader passed to ModelInstance::SetMaterialData is nullptr." );

		node_material_array.resize( model->MeshInstanceCount() );
		blinn_phong_material_data_array.resize( model->MeshInstanceCount() );

		const auto  node_count = model->NodeCount();
		const auto& nodes      = model->Nodes();

		for( auto& node : nodes )
		{
			if( node.mesh_group ) // Only process Nodes with Meshes.
			{
				for( auto& mesh_info : node.mesh_group->mesh_infos )
				{
					auto& material = node_material_array[ material_index ] = Material( model->Name() + "_" + mesh_info.name + "_" + std::to_string( material_index ),
																					   shader );

					if( mesh_info.texture_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = {},
							.has_texture_diffuse = 1,
							.shininess           = 32.0f
						};

						material.SetTexture( "uniform_tex_diffuse", mesh_info.texture_albedo );
					}
					else if( mesh_info.color_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = *mesh_info.color_albedo,
							.has_texture_diffuse = 0,
							.shininess           = 32.0f
						};
					}

					const RHI::Texture* default_normal_map_texture = BuiltinTextures::Get( "Normal Map" );
					const RHI::Texture* white_texture              = BuiltinTextures::Get( "White" );

					material.SetTexture( "uniform_tex_normal", mesh_info.texture_normal ? mesh_info.texture_normal : default_normal_map_texture );
					material.SetTexture( "uniform_tex_specular", white_texture );

					material.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );

					material_index++;
				}
			}
		}

		for( auto i = 0; i < blinn_phong_material_data_array.size(); i++ )
			node_material_array[ i ].Set( "BlinnPhongMaterialData", blinn_phong_material_data_array[ i ] );
	}

	void ModelInstance::ToggleShadowReceivingStatus( const bool receive_shadows )
	{
		if( receive_shadows == node_renderable_array.front().is_receiving_shadows )
			return;

		ASSERT_EDITOR_ONLY( shader && "Shader of the model instance is nullptr." );
		ASSERT_EDITOR_ONLY( shader_shadow_receiving && "Shader (shadow receiving) of the model instance is nullptr." );

		i32 material_index = 0;

		ASSERT_EDITOR_ONLY( shader && "Shader passed to ModelInstance::SetMaterialData is nullptr." );

		node_material_array.resize( model->MeshInstanceCount() );
		blinn_phong_material_data_array.resize( model->MeshInstanceCount() );

		const auto& nodes = model->Nodes();

		const auto shader_to_set = receive_shadows ? shader_shadow_receiving : shader;
	
		for( auto& node : nodes )
		{
			if( node.mesh_group ) // Only process Nodes with Meshes.
			{
				for( auto& mesh_info : node.mesh_group->mesh_infos )
				{
					auto& material = node_material_array[ material_index ];
					material.SetShader( shader_to_set );

					if( mesh_info.texture_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = {},
							.has_texture_diffuse = 1,
							.shininess           = 32.0f
						};

						material.SetTexture( "uniform_tex_diffuse", mesh_info.texture_albedo );
					}
					else if( mesh_info.color_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = *mesh_info.color_albedo,
							.has_texture_diffuse = 0,
							.shininess           = 32.0f
						};
					}

					const RHI::Texture* default_normal_map_texture = BuiltinTextures::Get( "Normal Map" );
					const RHI::Texture* white_texture              = BuiltinTextures::Get( "White" );

					material.SetTexture( "uniform_tex_normal", mesh_info.texture_normal ? mesh_info.texture_normal : default_normal_map_texture );
					material.SetTexture( "uniform_tex_specular", white_texture );

					material.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );

					material_index++;
				}
			}
		}

		for( auto i = 0; i < blinn_phong_material_data_array.size(); i++ )
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