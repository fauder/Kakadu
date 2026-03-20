#pragma once

// Engine Includes.
#include "SortingMode.h"
#include "RHI/Enums.h"

namespace Kakadu
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

		/* 2 bytes of padding here. */

	/* Face-culling & winding-order: */

		RHI::Face face_culling_face_to_cull          = RHI::Face::Back;
		RHI::WindingOrder face_culling_winding_order = RHI::WindingOrder::Clockwise;

	/* Depth: */

		RHI::ComparisonFunction depth_comparison_function = RHI::ComparisonFunction::Less;

		/* Stencil: */

		u32 stencil_write_mask                              = true;
		RHI::ComparisonFunction stencil_comparison_function = RHI::ComparisonFunction::Always;
		u32 stencil_ref                                     = 0;
		u32 stencil_mask                                    = 0xFF;

		RHI::StencilTestResponse stencil_test_response_stencil_fail            = RHI::StencilTestResponse::Keep;
		RHI::StencilTestResponse stencil_test_response_stencil_pass_depth_fail = RHI::StencilTestResponse::Keep;
		RHI::StencilTestResponse stencil_test_response_both_pass               = RHI::StencilTestResponse::Keep;

	/* Blending: */

		RHI::BlendingFactor blending_source_color_factor      = RHI::BlendingFactor::One;
		RHI::BlendingFactor blending_destination_color_factor = RHI::BlendingFactor::Zero;
		RHI::BlendingFactor blending_source_alpha_factor      = RHI::BlendingFactor::One;
		RHI::BlendingFactor blending_destination_alpha_factor = RHI::BlendingFactor::Zero;

		RHI::BlendingFunction blending_function = RHI::BlendingFunction::Add;

	}; /* Total: 2 bytes of padding. */
}
