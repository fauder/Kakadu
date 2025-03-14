#pragma once

// Engine Includes.
#include "Renderable.h"
#include "RenderState.h"

namespace Engine
{
	/* Forward declarations: */
	class Renderer;

	struct RenderQueue
	{
		using ReferenceCount = unsigned int;

		enum class ID : std::uint16_t {};

		std::string name = "<unnamed-queue>";

		/* Default behaviour for a queue is to not override source/target framebuffers and simply use the owning RenderPass' target framebuffer for rendering. */

		Framebuffer* framebuffer_override_source = nullptr; // Read.
		Framebuffer* framebuffer_override_target = nullptr; // Write.

		RenderState render_state_override;

		/* 4 bytes of padding. */

		std::vector< Renderable* > renderable_list;

		std::unordered_map< Shader*, ReferenceCount > shader_reference_counts;
		std::map< std::string, Shader* > shaders_in_flight;

		std::map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		bool is_enabled = true;

		/* 7 bytes of padding. */

	private:
		friend class Renderer;
	};
}
