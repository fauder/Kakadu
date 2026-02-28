#pragma once

// Engine Includes.
#include "Math/Vector.hpp"

// std Includes.
#include <cstdint>
#include <string>

// Vendor Includes
#include <imgui.h>

namespace Engine::Editor
{
	/* Forward Declarations: */
	struct Context;

	struct ViewportPanel
	{
		bool IsMouseHoveringOver() const { return is_hovered; }
		/* This returns the viewport coordinates (bottom-left origin for OpenGL). Beware: may return garbage when the mouse is outside the viewport. */
		Vector2 GetMouseScreenSpacePosition() const;
		void SetMagnifierZoomFactor( const std::uint8_t new_zoom_factor );
		void OffsetMagnifierZoomFactor( const bool increment );

		void Render( Context& editor_context, const unsigned int viewport_texture_id, const Vector2I viewport_framebuffer_size );

		static constexpr std::uint8_t SMALLEST_MAGNIFIER_ZOOM_FACTOR = 4;
		static constexpr std::uint8_t  LARGEST_MAGNIFIER_ZOOM_FACTOR = 32;

		std::string imgui_window_name;
		ImVec2 framebuffer_size; // The OpenGL framebuffer size.
		ImVec2 position_absolute; // The ImGui window position_absolute.
		Vector2I mouse_relative_position; // Screen-space position of the mouse, relative to OpenGL convention: the bottom-left of the viewport.

		std::uint8_t magnifier_zoom_factor = SMALLEST_MAGNIFIER_ZOOM_FACTOR;
		bool is_hovered = false;
		/* 6 bytes(s) of padding. */
	};
}
