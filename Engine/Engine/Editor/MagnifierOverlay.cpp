#define IMGUI_DEFINE_MATH_OPERATORS

// Engine Includes.
#include "MagnifierOverlay.h"
#include "Core/ImGuiUtility.h"
#include "Math/VectorConversion.hpp"

namespace Engine::Editor
{
	void RenderMagnifierOverlay( Context& context, const unsigned int viewport_texture_id )
	{
		ViewportPanel& viewport_panel = context.viewport_panel;

		const float zoom = ( float )viewport_panel.magnifier_zoom_factor;
		const float window_size = 256.0f * Math::Max( 1.0f, Math::Log2( zoom ) - 1 );

		ASSERT( zoom >= 1.0f );

		const auto& style = ImGui::GetStyle();

		// Find out how many "original pixels" a magnifier pixel is:
		const float magnified_pixel_multiplier = 1.0f / zoom;

		// Calculate UV coordinates in the viewport texture:
		ImVec2 mouse_pos = Math::ToImVec2( viewport_panel.GetMouseScreenSpacePosition() + Vector2{ 0.5f, 0.5f } );
		ImVec2 uv_center = mouse_pos / viewport_panel.framebuffer_size;
		ImVec2 uv_radius = ImVec2( 0.5f * window_size * magnified_pixel_multiplier,
								   0.5f * window_size * magnified_pixel_multiplier ) / viewport_panel.framebuffer_size;

		ImVec2 uv0 = uv_center - uv_radius;
		ImVec2 uv1 = uv_center + uv_radius;

		// Clamp to [0,1] to avoid wrapping:
		uv0.x = Math::Clamp( uv0.x, 0.0f, 1.0f );
		uv0.y = Math::Clamp( uv0.y, 0.0f, 1.0f );
		uv1.x = Math::Clamp( uv1.x, 0.0f, 1.0f );
		uv1.y = Math::Clamp( uv1.y, 0.0f, 1.0f );

		std::swap( uv0.y, uv1.y );

		viewport_panel.mouse_relative_position = Vector2I( viewport_panel.GetMouseScreenSpacePosition() );
		const auto imgui_mouse_pos = ImGui::GetMousePos() + ImVec2( 5, 5 );

		if( ImGuiUtility::BeginOverlay( viewport_panel.imgui_window_name.c_str(), "##Magnifier", imgui_mouse_pos, &context.mouse_screen_space_position_overlay_is_active,
										/* No input: */ false ) )
		{
			const auto cursor_pos_before_image( ImGui::GetCursorScreenPos() );
			ImGui::Image( ( ImTextureID )viewport_texture_id, ImVec2( window_size, window_size ), uv0, uv1 );

			/* Show center pixel outline: */
			ImDrawList* draw_list = ImGui::GetWindowDrawList();

			const auto center_pos = cursor_pos_before_image + ImVec2( window_size / 2, window_size / 2 );
			const auto thickness = zoom / 8;

			ImVec2 rect_min = center_pos - ImVec2( 1, 1 ) * ( ( zoom + thickness ) / 2 );
			ImVec2 rect_max = center_pos + ImVec2( 1, 1 ) * ( ( zoom + thickness ) / 2 );

			draw_list->AddRect( rect_min, rect_max, IM_COL32( 255, 0, 0, 255 ), zoom / 5, 0, zoom / 8 );
		}

		Engine::ImGuiUtility::EndOverlay();

		/* Display zoom level in a small centered overlay: */
		{
			char zoom_label[ 16 ];
			sprintf_s( zoom_label, "%.0fx", zoom );

			ImVec2 text_size = ImGui::CalcTextSize( zoom_label );

			if( ImGuiUtility::BeginOverlay( viewport_panel.imgui_window_name.c_str(),
											"##MagnifierLabel",
											imgui_mouse_pos + ImVec2( ( window_size - text_size.x ) * 0.5f, window_size + style.WindowPadding.y * 2.0f + 2 ),
											nullptr,
											/* No input: */ false ) )
			{
				ImGui::TextUnformatted( zoom_label );
			}

			ImGuiUtility::EndOverlay();
		}

		/* Display mouse position in a small overlay above the magnifier overlay: */
		{
			const auto imgui_mouse_pos = ImGui::GetMousePos() + ImVec2( 5, -( ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y * 2 ) );

			if( Engine::ImGuiUtility::BeginOverlay( viewport_panel.imgui_window_name.c_str(), "##Fragment Pos.", imgui_mouse_pos, &context.mouse_screen_space_position_overlay_is_active,
													/* No input: */ false ) )
				ImGui::TextDisabled( "(%d, %d)", viewport_panel.mouse_relative_position.X(), viewport_panel.mouse_relative_position.Y() );

			Engine::ImGuiUtility::EndOverlay();
		}
	}
}
