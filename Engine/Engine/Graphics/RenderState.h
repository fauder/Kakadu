#pragma once

// Engine Includes.
#include "Enums.h"

namespace Engine
{
	struct RenderState
	{
	/* Enable flags (kept together for optimal memory alignment): */

		bool face_culling_enable = true; // Differing from GL here; Back face culling is the default, to save perf.
		bool depth_test_enable   = true;
		bool depth_write_enable  = true;
		bool stencil_test_enable = false;
		bool blending_enable     = false;

	/* Sorting: */

		SortingMode sorting_mode = SortingMode::FrontToBack;

	/* Polygon Mode: */

		PolygonMode polygon_mode = PolygonMode::Fill;

		/* 1 byte(s) of padding here. */

	/* Face-culling & winding-order: */

		Face face_culling_face_to_cull          = Face::Back;
		WindingOrder face_culling_winding_order = WindingOrder::Clockwise;

	/* Depth: */

		ComparisonFunction depth_comparison_function = ComparisonFunction::Less;

	/* Stencil: */

		unsigned int stencil_write_mask                = true;
		ComparisonFunction stencil_comparison_function = ComparisonFunction::Always;
		unsigned int stencil_ref                       = 0;
		unsigned int stencil_mask                      = 0xFF;

		StencilTestResponse stencil_test_response_stencil_fail            = StencilTestResponse::Keep;
		StencilTestResponse stencil_test_response_stencil_pass_depth_fail = StencilTestResponse::Keep;
		StencilTestResponse stencil_test_response_both_pass               = StencilTestResponse::Keep;

	/* Blending: */

		BlendingFactor blending_source_color_factor      = BlendingFactor::One;
		BlendingFactor blending_destination_color_factor = BlendingFactor::Zero;
		BlendingFactor blending_source_alpha_factor      = BlendingFactor::One;
		BlendingFactor blending_destination_alpha_factor = BlendingFactor::Zero;

		BlendingFunction blending_function = BlendingFunction::Add;
	};
}
