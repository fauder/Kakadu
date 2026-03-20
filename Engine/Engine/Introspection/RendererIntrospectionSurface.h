#pragma once

// Engine Includes.
#include "Graphics/FullscreenEffect.h"
#include "Graphics/RenderPass.h"
#include "Graphics/RenderQueue.h"
#include "Graphics/RHI/Framebuffer.h"
#include "Math/OrthographicProjectionParameters.h"

// std Includes.
#include <map>

namespace Kakadu
{
	struct RendererIntrospectionSurface
	{
		std::vector< RHI::Framebuffer >* framebuffers;

		std::map< RenderPass::ID,  RenderPass  >* render_pass_map;
		std::map< RenderQueue::ID, RenderQueue >* render_queue_map;

		FullscreenEffect* msaa_resolve;
		std::map< std::string, FullscreenEffect* >* post_processing_effect_map;
		FullscreenEffect* tone_mapping;

		OrthographicProjectionParameters* shadow_mapping_projection_parameters;

		Material* skybox_material;

		UniformBufferManagement< DirtyBlob >* uniform_buffer_management_global;
		UniformBufferManagement< DirtyBlob >* uniform_buffer_management_intrinsic;
	};
}
