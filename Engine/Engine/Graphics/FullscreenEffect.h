#pragma once

// Engine Includes.
#include "Framebuffer.h"
#include "Material.hpp"
#include "Renderable.h"
#include "RenderState.h"

// std Includes.
#include <vector>

namespace Engine
{
	class Renderer;

	struct FullscreenEffect
	{
		static constexpr RenderState DefaultRenderState = RenderState
		{
			// Most of the state is omitted here as the default values are mostly correct for a post-processing effect.

			.face_culling_enable = true,  // No point in rendering the back-face of a full-screen quad.
			.depth_test_enable   = false, // No point in depth desting as there is only 1 geometry.
			.depth_write_enable  = false, // No point in depth desting as there is only 1 geometry.

			.sorting_mode = SortingMode::None, // Preserve insertion order.
		};

		struct Step
		{
			Framebuffer* framebuffer_target;
			const Texture* texture_input;
		};

		std::string name;

		std::vector< Step > steps;
		Material material;

		std::vector< Framebuffer > framebuffers;

		/* If this is not set, Renderer will default-execute the effect. */
		std::function< void( Renderer& renderer ) > execution_routine;

		RenderState render_state = DefaultRenderState;

		bool is_enabled = true;

		// 3 bytes of padding.
	};
}
