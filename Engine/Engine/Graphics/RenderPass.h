#pragma once

// Engine Includes.
#include "Framebuffer.h"
#include "RenderQueue.h"

// std Includes.
#include <optional>
#include <set>

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	struct RenderPass
	{
		enum class ID : std::uint8_t {};

		std::string name = "<unnamed-pass>";

		Framebuffer* target_framebuffer; // nullptr => Default framebuffer.

		std::set< RenderQueue::ID > queue_id_set;

		std::optional< Matrix4x4 > view_matrix;
		std::optional< Matrix4x4 > projection_matrix;
		
		float plane_near;
		float plane_far;
		float aspect_ratio;
		Radians vertical_field_of_view;

		RenderState render_state;

		bool render_state_override_is_allowed = true;
		bool is_enabled                       = true;
		bool clear_framebuffer                = true;
		
		/* 1 byte padding. */

	private:
		friend class Renderer;
	};
}
