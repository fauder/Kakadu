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

		RenderState render_state_override;

		/* 4 bytes of padding. */

		std::vector< Renderable* > renderable_list;

		std::unordered_map< Shader*, ReferenceCount > shaders_in_flight;
		std::unordered_map< std::string, Material* > materials_in_flight; // TODO: Generate an ID for Materials (who will generate it?) and use that ID as the key here.

		bool is_enabled = true;

		/* 7 bytes of padding. */

	private:
		friend class Renderer;
	};
}
