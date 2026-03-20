#pragma once

// Engine Includes.
#include "MaterialData/MaterialData.h"
#include "Model.h"
#include "Renderer.h"

// std Includes.
#include <vector>

namespace Kakadu
{
	class ModelInstance
	{
	public:
		ModelInstance();
		ModelInstance( const Model* model,
					   RHI::Shader* const shader,
					   RHI::Shader* const shader_shadow_receiving,
					   const Vector3 scale, const Quaternion& rotation, const Vector3& translation,
					   Material* material,
					   const bool receives_shadows            = false,
					   const bool casts_shadows               = false,
					   const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

		DELETE_COPY_CONSTRUCTORS( ModelInstance );
		DEFAULT_MOVE_CONSTRUCTORS( ModelInstance );

		~ModelInstance();

		/*
		 * Queries:
		 */

		bool IsReceivingShadows()	const { return node_renderable_array.front().is_receiving_shadows; }
		bool IsCastingShadows()		const { return node_renderable_array.front().is_casting_shadows; }

		void SetMaterialData( RHI::Shader* const shader, const Vector4 texture_scale_and_offset = Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

		void ToggleShadowReceivingStatus( const bool receive_shadows );
		void ToggleShadowCastingStatus( const bool cast_shadows );

	/* Queries: */
			  std::vector< Renderable >& Renderables()		 { return node_renderable_array; }
		const std::vector< Material	  >& Materials()   const { return node_material_array; }

	private:
		const Model* model;

		RHI::Shader* shader;
		RHI::Shader* shader_shadow_receiving;

		std::vector< Renderable > node_renderable_array;
		std::vector< Material	> node_material_array;
		std::vector< Transform	> node_transform_array;
		std::vector< MaterialData::BlinnPhongMaterialData > blinn_phong_material_data_array;
		
		Vector4 texture_scale_and_offset;
	};
}
