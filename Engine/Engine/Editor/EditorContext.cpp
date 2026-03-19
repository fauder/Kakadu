// Engine Includes.
#include "EditorContext.h"
#include "FrameStatisticsOverlay.h"
#include "ViewportControlsOverlay.h"
#include "RendererPanel.h"
#include "SceneCameraInspectorPanel.h"
#include "Core/AssetDatabase.hpp"
#include "Core/AssetDatabase_Tracked.hpp"
#include "Core/ImGuiDrawer.hpp"
#include "Core/ImGuiSetup.h"
#include "Core/MorphSystem.h"
#include "Core/ServiceLocator.h"
#include "Graphics/Renderer.h"

// TODO: Add the menu bar => and a window entry => ability to close/reopen desired panels.

namespace Kakadu::Editor
{
	internal_variable FrameStatisticsOverlay	FRAME_STATS_OVERLAY;
	internal_variable RendererPanel				RENDERER_PANEL;

	internal_function void EnableShaderHotReloading( Renderer* renderer )
	{
		ServiceLocator< MorphSystem >::Get().Add( Morph
												  {
													  .on_complete = [ renderer ]()
													  {
														  renderer->RecompileModifiedShaders();
													  },
													  .duration_in_seconds = 0.1f, // Seems enough.
													  .is_looping = true,
													  .use_real_time = true
												  } );
	}

	void Context::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		switch( key_code )
		{
			case Platform::KeyCode::KEY_ESCAPE:
				if( show_mouse_screen_space_position_overlay )
					show_mouse_screen_space_position_overlay = false;
				break;
				/* Use the key below ESC to toggle between game & menu/UI. */
			case Platform::KeyCode::KEY_GRAVE_ACCENT:
				if( key_action == Platform::KeyAction::PRESS )
				{
					ui_interaction_enabled = not ui_interaction_enabled;
					Platform::ResetMouseDeltas();
				}
				break;
			case Platform::KeyCode::KEY_I:
				if( key_action == Platform::KeyAction::PRESS )
					show_imgui_demo_window = not show_imgui_demo_window;
				break;
			case Platform::KeyCode::KEY_F11:
				if( key_action == Platform::KeyAction::PRESS &&
					viewport_panel.is_hovered )
				{
					show_imgui = not show_imgui;

					commands_queue.push( Command{ .type = Command::Type::Renderer_ToggleViewportRenderTarget } );
				}
				break;
			default:
				break;
		}
	}

	void Context::OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
	{
	}

	void Context::OnMouseScrollEvent( const float x_offset, const float y_offset )
	{
		/* Activate/deactivate magnifier on zoom start/exit: */
		if( y_offset > 0 && not show_mouse_screen_space_position_overlay )
		{
			show_mouse_screen_space_position_overlay = true;
			viewport_panel.magnifier_zoom_factor = ViewportPanel::SMALLEST_MAGNIFIER_ZOOM_FACTOR;
		}
		else if( y_offset < 0 && show_mouse_screen_space_position_overlay && viewport_panel.magnifier_zoom_factor == ViewportPanel::SMALLEST_MAGNIFIER_ZOOM_FACTOR )
			show_mouse_screen_space_position_overlay = false;
		else if( show_mouse_screen_space_position_overlay )
			viewport_panel.OffsetMagnifierZoomFactor( y_offset > 0 );
	}

	void Context::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
	{
		scene_camera.RecalculateProjectionParameters( width_new_pixels, height_new_pixels );
	}

	void Context::Initialize()
	{
		EnableShaderHotReloading( renderer );
	}

	void Context::Update( const FrameTime& frame_time )
	{
		if( mouse_screen_space_position_overlay_is_active )
		{
			if( Platform::IsKeyPressedThisFrame( Platform::KeyCode::KEY_LEFT ) )
				Platform::OffsetMouseCursorPosition( -1.0f, 0.0f );
			if( Platform::IsKeyPressedThisFrame( Platform::KeyCode::KEY_RIGHT ) )
				Platform::OffsetMouseCursorPosition( +1.0f, 0.0f );
			if( Platform::IsKeyPressedThisFrame( Platform::KeyCode::KEY_DOWN ) )
				Platform::OffsetMouseCursorPosition( 0.0f, +1.0f );
			if( Platform::IsKeyPressedThisFrame( Platform::KeyCode::KEY_UP ) )
				Platform::OffsetMouseCursorPosition( 0.0f, -1.0f );
		}

		scene_camera.Update( frame_time.time_current, frame_time.time_delta, not ui_interaction_enabled );

		FRAME_STATS_OVERLAY.Update( frame_time.time_delta_real );
	}

	void Context::RenderUI()
	{
		if( not show_imgui )
			return;

		if( show_imgui_demo_window )
			ImGui::ShowDemoWindow();

		ImGui::DockSpaceOverViewport();

		ImGuiDrawer::Update();

		RENDERER_PANEL.Render( *renderer, renderer_introspection_surface );

		{
			const auto& framebuffer_color_attachment = renderer->OutputFramebuffer().color_attachment;
			viewport_panel.Render( *this, framebuffer_color_attachment.Id().id, framebuffer_color_attachment.Size() );
		}

		RenderViewportControlsOverlay( *this, *renderer );

		FRAME_STATS_OVERLAY.Render( *this );

		if( show_gl_logger )
			ServiceLocator< GLLogger >::Get().Draw( &show_gl_logger );

		RenderSceneCameraInspectorPanel( scene_camera, viewport_resolution );

		ImGuiDrawer::Draw( ServiceLocator< AssetDatabase< Texture > >::Get().Assets() );
		ImGuiDrawer::Draw( ServiceLocator< AssetDatabase_Tracked< Texture* > >::Get().Assets() );

		auto log_group( ServiceLocator< GLLogger >::Get().TemporaryLogGroup( "ImGuiSetup::EndFrame()" ) );
	}
}
