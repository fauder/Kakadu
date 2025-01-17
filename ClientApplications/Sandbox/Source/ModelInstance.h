#pragma once

// Engine Includes.
#include <Engine/DefineMathTypes.h>
#include <Engine/Graphics/MaterialData/MaterialData.h>
#include <Engine/Graphics/Model.h>
#include <Engine/Graphics/Renderer.h>
#include <Engine/Math/Matrix.h>

// std Includes.
#include <map>
#include <vector>

class ModelInstance
{
	DEFINE_MATH_TYPES()

public:
	ModelInstance();
	ModelInstance( const Engine::Model* model, Engine::Shader* const shader,
				   const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
				   const Engine::RenderQueue::ID queue_id,
				   Engine::Material* material,
				   const bool has_shadows = false,
				   const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	DELETE_COPY_CONSTRUCTORS( ModelInstance );
	DEFAULT_MOVE_CONSTRUCTORS( ModelInstance );

	~ModelInstance();

	void SetMaterialData( Engine::Shader* const shader, const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

/* Queries: */
	inline		 std::vector< Engine::Renderable >& Renderables()			{ return node_renderable_array; }
	inline const std::vector< Engine::Material	 >& Materials()		const	{ return node_material_array; }

private:
	const Engine::Model* model;
	Engine::RenderQueue::ID queue_id;
	std::vector< Engine::Renderable > node_renderable_array;
	std::vector< Engine::Material > node_material_array;
	std::vector< Engine::Transform > node_transform_array;
	std::vector< Engine::MaterialData::BlinnPhongMaterialData > blinn_phong_material_data_array;
};
