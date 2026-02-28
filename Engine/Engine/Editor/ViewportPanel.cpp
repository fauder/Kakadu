#define IMGUI_DEFINE_MATH_OPERATORS

// Editor Includes.
#include "ViewportPanel.h"
#include "EditorContext.h"
#include "MagnifierOverlay.h"
#include "Core/Platform.h"
#include "Math/VectorConversion.hpp"

namespace Engine::Editor
{
	/* This returns the viewport coordinates (bottom-left origin for OpenGL). Beware: may return garbage when the mouse is outside the viewport. */
	Vector2 ViewportPanel::GetMouseScreenSpacePosition() const
	{
		ImVec2 mouse = ImGui::GetMousePos();

		/* ImGui uses top-left as the origin for its windows while OpenGL's viewport conventions dictate bottom-left as the origin => flip Y.
		 * Relative mouse pos: */
		return Vector2( mouse.x - position_absolute.x, framebuffer_size.y - ( mouse.y - position_absolute.y ) );
	}

	void ViewportPanel::SetMagnifierZoomFactor( const std::uint8_t new_zoom_factor )
	{
		magnifier_zoom_factor = Math::Clamp( new_zoom_factor, SMALLEST_MAGNIFIER_ZOOM_FACTOR, LARGEST_MAGNIFIER_ZOOM_FACTOR );
	}

	void ViewportPanel::OffsetMagnifierZoomFactor( const bool increment )
	{
		SetMagnifierZoomFactor( increment
								? magnifier_zoom_factor << 1
								: magnifier_zoom_factor >> 1 );
	}

	void ViewportPanel::Render( Context& editor_context, const unsigned int viewport_texture_id, const Vector2I viewport_framebuffer_size )
	{
		{
			const auto framebuffer_size = Platform::GetFramebufferSizeInPixels();
			ImGui::SetNextWindowSize( Math::CopyToImVec2( framebuffer_size ), ImGuiCond_FirstUseEver );
		}

		imgui_window_name = std::format( "Viewport {:d}x{:d}###Viewport", ( int )framebuffer_size.x, ( int )framebuffer_size.y );

		if( ImGui::Begin( imgui_window_name.c_str() ) )
		{
			framebuffer_size = ImGui::GetContentRegionAvail();
			const Vector2I viewport_available_size( ( int )framebuffer_size.x, ( int )framebuffer_size.y );

			const auto& imgui_io = ImGui::GetIO();
			if( viewport_available_size != viewport_framebuffer_size &&
				( not imgui_io.WantCaptureMouse || not imgui_io.MouseDown[ 0 ] ) )
			{
				editor_context.commands_queue.push( Command
													{
														.type    = Command::Type::Renderer_HandlePendingViewportResize,
														.payload = std::bit_cast< decltype( Command::payload ) >( viewport_available_size )
													} );
			}

			/* Collect information for mouse hover/pos. info detection OUTSIDE the Begin()/End() block here. */
			if( is_hovered = ImGui::IsWindowHovered() )
			{
				/* If the Viewport is hovered, ImGui should NOT consume input and let the Application handle it: */
				ImGui::SetNextFrameWantCaptureMouse( false );
				ImGui::SetNextFrameWantCaptureKeyboard( false );
			}

			position_absolute = ImGui::GetCursorScreenPos();

			if( editor_context.show_mouse_screen_space_position_overlay )
			{
				if( editor_context.mouse_screen_space_position_overlay_is_active = is_hovered && ImGui::IsWindowFocused( ImGuiFocusedFlags_ChildWindows ) )
				{
					RenderMagnifierOverlay( editor_context, viewport_texture_id );
				}
			}

			ImGui::Image( ( ImTextureID )viewport_texture_id, ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
		}

		ImGui::End();
	}
}
