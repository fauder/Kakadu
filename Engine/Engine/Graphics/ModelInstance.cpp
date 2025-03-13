// Engine Includes.
#include "ModelInstance.h"
#include "Graphics/BuiltinTextures.h"

namespace Engine
{
	ModelInstance::ModelInstance()
		:
		model( nullptr ),
		shader( nullptr ),
		shader_shadow_receiving( nullptr )
	{}

	ModelInstance::ModelInstance( const Engine::Model* model, 
								  Engine::Shader* const shader,
								  Engine::Shader* const shader_shadow_receiving,
								  const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
								  Engine::Material* material,
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

		for( auto i = 0; i < mesh_instance_count; i++ )
		{
			node_renderable_array[ i ] = Engine::Renderable( &meshes[ i ], ( material ? material : &node_material_array[ i ] ),
															 node_material_array[ i ].HasUniform( "uniform_transform_world" ) ? &node_transform_array[ i ] : nullptr,
															 receives_shadows, casts_shadows );
		}

		/* Apply scene-graph transformations: */

		int mesh_index = 0;
		std::function< void( const std::size_t, const Matrix4x4& ) > ProcessNode = [ & ]( const std::size_t node_index, const Matrix4x4& parent_transform )
		{
			const auto& node = nodes[ node_index ];

			auto transform_so_far = node.transform_local * parent_transform;

			for( auto& child_index : node.children )
				ProcessNode( child_index, transform_so_far );

			if( node.mesh_group )
				for( auto& sub_mesh : node.mesh_group->sub_meshes )
					node_transform_array[ mesh_index++ ].SetFromSRTMatrix( transform_so_far );
		};

		for( auto top_level_node_index : model->TopLevelNodeIndices() )
			ProcessNode( top_level_node_index, Engine::Matrix::Scaling( scale ) * Engine::Math::QuaternionToMatrix( rotation ) * Engine::Matrix::Translation( translation ) );
	}

	ModelInstance::~ModelInstance()
	{}

	void ModelInstance::SetMaterialData( Engine::Shader* const shader, const Vector4 texture_scale_and_offset )
	{
		int material_index = 0;

		ASSERT_EDITOR_ONLY( shader && "Shader passed to ModelInstance::SetMaterialData is nullptr." );

		node_material_array.resize( model->MeshInstanceCount() );
		blinn_phong_material_data_array.resize( model->MeshInstanceCount() );

		const auto  node_count = model->NodeCount();
		const auto& nodes      = model->Nodes();

		for( auto& node : nodes )
		{
			if( node.mesh_group ) // Only process Nodes with Meshes.
			{
				for( auto& sub_mesh : node.mesh_group->sub_meshes )
				{
					auto& material = node_material_array[ material_index ] = Engine::Material( model->Name() + "_" + sub_mesh.name, shader );

					if( sub_mesh.texture_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = {},
							.has_texture_diffuse = 1,
							.shininess           = 32.0f
						};

						material.SetTexture( "uniform_tex_diffuse", sub_mesh.texture_albedo );
					}
					else if( sub_mesh.color_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = *sub_mesh.color_albedo,
							.has_texture_diffuse = 0,
							.shininess           = 32.0f
						};
					}

					static const auto default_normal_map_texture = Engine::BuiltinTextures::Get( "Normal Map" );
					static const auto white_texture              = Engine::BuiltinTextures::Get( "White" );

					material.SetTexture( "uniform_tex_normal", sub_mesh.texture_normal ? sub_mesh.texture_normal : default_normal_map_texture );
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
		if( receive_shadows == node_renderable_array.front().IsReceivingShadows() )
			return;

		ASSERT_EDITOR_ONLY( shader && "Shader of the model instance is nullptr." );
		ASSERT_EDITOR_ONLY( shader_shadow_receiving && "Shader (shadow receiving) of the model instance is nullptr." );

		int material_index = 0;

		ASSERT_EDITOR_ONLY( shader && "Shader passed to ModelInstance::SetMaterialData is nullptr." );

		node_material_array.resize( model->MeshInstanceCount() );
		blinn_phong_material_data_array.resize( model->MeshInstanceCount() );

		const auto& nodes = model->Nodes();

		const auto shader_to_set = receive_shadows ? shader_shadow_receiving : shader;
	
		for( auto& node : nodes )
		{
			if( node.mesh_group ) // Only process Nodes with Meshes.
			{
				for( auto& sub_mesh : node.mesh_group->sub_meshes )
				{
					auto& material = node_material_array[ material_index ];
					material.SetShader( shader_to_set );

					if( sub_mesh.texture_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = {},
							.has_texture_diffuse = 1,
							.shininess           = 32.0f
						};

						material.SetTexture( "uniform_tex_diffuse", sub_mesh.texture_albedo );
					}
					else if( sub_mesh.color_albedo )
					{
						blinn_phong_material_data_array[ material_index ] =
						{
							.color_diffuse       = *sub_mesh.color_albedo,
							.has_texture_diffuse = 0,
							.shininess           = 32.0f
						};
					}

					static const auto default_normal_map_texture = Engine::BuiltinTextures::Get( "Normal Map" );
					static const auto white_texture              = Engine::BuiltinTextures::Get( "White" );

					material.SetTexture( "uniform_tex_normal", sub_mesh.texture_normal ? sub_mesh.texture_normal : default_normal_map_texture );
					material.SetTexture( "uniform_tex_specular", white_texture );

					material.Set( "uniform_texture_scale_and_offset", texture_scale_and_offset );

					material_index++;
				}
			}
		}

		for( auto i = 0; i < blinn_phong_material_data_array.size(); i++ )
			node_material_array[ i ].Set( "BlinnPhongMaterialData", blinn_phong_material_data_array[ i ] );

		for( auto& renderable : node_renderable_array )
			renderable.ToggleShadowReceiving( receive_shadows );
	}

	void ModelInstance::ToggleShadowCastingStatus( const bool cast_shadows )
	{
		for( auto& renderable : node_renderable_array )
			renderable.ToggleShadowCasting( cast_shadows );
	}
}