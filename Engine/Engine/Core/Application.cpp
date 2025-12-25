#define IMGUI_DEFINE_MATH_OPERATORS

// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiDrawer.hpp"
#include "ImGuiSetup.h"
#include "ImGuiUtility.h"
#include "Graphics/Enums.h"
#include "Math/Math.hpp"
#include "Math/VectorConversion.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <Tracy/tracy/Tracy.hpp>
#include <Tracy/tracy/TracyOpenGL.hpp>

/* To enable some .natvis functionality: */
#include "Natvis/Natvis.h"

// std Includes.
#include <iostream>

namespace Engine
{
	Application::Application( const BitFlags< CreationFlags > flags,
							  Renderer::Description&& renderer_description )
		:
#ifdef _EDITOR
		show_frame_statistics_overlay( true ),
		show_mouse_screen_space_position_overlay( false ),
		show_imgui( not flags.IsSet( CreationFlags::OnStart_DisableImGui ) ),
		show_imgui_demo_window( false ),
		show_gl_logger( true ),
		ui_interaction_enabled( true ),
#endif // _EDITOR
		gamma_correction_is_enabled( renderer_description.enable_gamma_correction && not flags.IsSet( CreationFlags::OnStart_DisableGammaCorrection ) ),
		vsync_is_enabled( false ),
		time_current( 0.0f ),
		time_multiplier( 1.0f ),
		frame_count( 1 ),
		msaa_sample_count( renderer_description.main_framebuffer_msaa_sample_count ),
		time_previous( 0.0f ),
		time_previous_since_start( 0.0f ),
		time_since_start( 0.0f ),
		mouse_screen_space_position_overlay_is_active( false )
	{
		NatVis::ForceIncludeInBuild();

		Initialize();

		renderer_description.enable_gamma_correction = gamma_correction_is_enabled;
		renderer = std::make_unique< Renderer >( std::move( renderer_description ) );

		scene_camera = std::make_unique< Editor::SceneCamera >(); // Needs to be initialized after the Platform layer is initialized to be able to query framebuffer size for aspect ratio.
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		ServiceLocator< GLLogger >::Register( &gl_logger );
		ServiceLocator< AssetDatabase< Texture > >::Register( &asset_database_texture );
		ServiceLocator< AssetDatabase_Tracked< Texture* > >::Register( &asset_database_texture_tracked );
		ServiceLocator< AssetDatabase< Model > >::Register( &asset_database_model );
		ServiceLocator< MorphSystem >::Register( &morph_system );

		Platform::InitializeAndCreateWindow( 800, 600, vsync_is_enabled );

		const auto version = glGetString( GL_VERSION );
		std::cout << version << "\n\n";

		ImGuiSetup::Initialize( gamma_correction_is_enabled );
		ImGuiDrawer::Initialize();

		Platform::SetKeyboardEventCallback(
			[ = ]( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
			{
				this->OnKeyboardEventInternal( key_code, key_action, key_mods );
			} );

		Platform::SetMouseButtonEventCallback(
			[ = ]( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
			{
				this->OnMouseButtonEventInternal( button, button_action, key_mods );
			} );

		Platform::SetMouseScrollEventCallback(
			[ = ]( const float x_offset, const float y_offset )
			{
				this->OnMouseScrollEventInternal( x_offset, y_offset );
			} );

		Platform::SetFramebufferResizeCallback(
			[ = ]( const int width_new_pixels, const int height_new_pixels )
			{
				this->OnFramebufferResizeEventInternal( width_new_pixels, height_new_pixels );
			} );

#ifdef _EDITOR
		Platform::SetGLDebugOutputCallback( gl_logger.GetCallback() );
#endif // _EDITOR
	}

	void Application::Shutdown()
	{
		ImGuiSetup::Shutdown();
	}

	void Application::Run()
	{
		TracyGpuContext;

		/* The render loop. */
		while( !Platform::ShouldClose() )
		{
			TracyGpuZone( "GPU Time" );

			CalculateTimeInformation();

			Platform::PollEvents();

			if( Platform::IsWindowIconified() )
			{
				Platform::Sleep( 0.2f );
				continue;
			}

			{
				ZoneScopedN( "Update" ); // Is (most probably) overridden in the client app. Makes sense to instrument here instead.
				Update();
			}

#ifdef _EDITOR
			{
				ZoneScopedN( "RenderViewport" );
				RenderViewport();
			}
#endif // _EDITOR


			{
				ZoneScopedN( "Render" ); // Is (most probably) overridden in the client app. Makes sense to instrument here instead.
				Render();
			}

			if( show_imgui )
			{
				ZoneScopedN( "ImGui" );
				ImGuiSetup::BeginFrame();
				RenderImGui();
				auto log_group( ServiceLocator< GLLogger >::Get().TemporaryLogGroup( "ImGuiSetup::EndFrame()" ) );
				ImGuiSetup::EndFrame();
			}

			{
				ZoneScopedN( "Swap Buffers" );
				Platform::SwapBuffers();
			}

			TracyGpuCollect;
			FrameMark;
		}
	}

	void Application::Update()
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

		morph_system.Execute( time_delta );

#ifdef _EDITOR
		scene_camera->Update( time_current, time_delta, not ui_interaction_enabled );
		renderer->Update();
#endif // _EDITOR
	}

	void Application::Render()
	{
		// Client App can override this to inject custom rendering code.
		// TODO: Implement actual game camera rendering.
	}

	void Application::RenderViewport()
	{
		renderer->UpdatePerPass( Engine::Renderer::RENDER_PASS_ID_LIGHTING, scene_camera->camera );

		renderer->Render();
	}

	void Application::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
#ifdef _EDITOR
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
					ui_interaction_enabled = !ui_interaction_enabled;
					Platform::ResetMouseDeltas();
				}
				break;
			case Platform::KeyCode::KEY_I:
				if( key_action == Platform::KeyAction::PRESS )
					show_imgui_demo_window = !show_imgui_demo_window;
				break;
			case Platform::KeyCode::KEY_F11:
				if( key_action == Platform::KeyAction::PRESS )
				{
					show_imgui = !show_imgui;
					if( show_imgui )
					{
						renderer->SetFinalPassToUseFinalFramebuffer();
					}
					else
					{
						OnFramebufferResizeEvent( Platform::GetFramebufferSizeInPixels() );
						renderer->SetFinalPassToUseDefaultFramebuffer();
					}
				}
				break;
			default:
				break;
		}
#endif // _EDITOR
	}

	void Application::OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
	{
	}

	void Application::OnMouseScrollEvent( const float x_offset, const float y_offset )
	{
		/* Activate/deactivate magnifier on zoom start/exit: */
		if( y_offset > 0 && not show_mouse_screen_space_position_overlay )
		{
			show_mouse_screen_space_position_overlay = true;
			viewport_info.magnifier_zoom_factor = ViewportWindowInfo::SMALLEST_MAGNIFIER_ZOOM_FACTOR;
		}
		else if( y_offset < 0 && show_mouse_screen_space_position_overlay && viewport_info.magnifier_zoom_factor == ViewportWindowInfo::SMALLEST_MAGNIFIER_ZOOM_FACTOR )
			show_mouse_screen_space_position_overlay = false;
		else if( show_mouse_screen_space_position_overlay )
			OffsetViewportMagnifierZoomFactor( y_offset > 0 );
	}

	void Application::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
	{
#ifdef _EDITOR
		scene_camera->RecalculateProjectionParameters( width_new_pixels, height_new_pixels );
#endif // _EDITOR

	}

	void Application::OnFramebufferResizeEvent( const Vector2I new_size_pixels )
	{
		OnFramebufferResizeEvent( new_size_pixels.X(), new_size_pixels.Y() );
	}

#ifdef _EDITOR
	Vector2 Application::GetMouseScreenSpacePosition() const
	{
		ImVec2 mouse = ImGui::GetMousePos();

		/* ImGui uses top-left as the origin for its windows while OpenGL's viewport conventions dictate bottom-left as the origin => flip Y.
		 * Relative mouse pos: */
		return Vector2( mouse.x - viewport_info.position_absolute.x, viewport_info.framebuffer_size.y - ( mouse.y - viewport_info.position_absolute.y ) );
	}

	void Application::SetViewportMagnifierZoomFactor( const std::uint8_t new_zoom_factor )
	{
		viewport_info.magnifier_zoom_factor = Math::Clamp( new_zoom_factor, ViewportWindowInfo::SMALLEST_MAGNIFIER_ZOOM_FACTOR, ViewportWindowInfo::LARGEST_MAGNIFIER_ZOOM_FACTOR );
	}

	void Application::OffsetViewportMagnifierZoomFactor( const bool increment )
	{
		SetViewportMagnifierZoomFactor( increment
											? viewport_info.magnifier_zoom_factor << 1
											: viewport_info.magnifier_zoom_factor >> 1 );
	}
#endif

	void Application::OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		// No need to query ImGui::GetIO().WantCaptureKeyboard here because Platform already queries it before calling this function.

		OnKeyboardEvent( key_code, key_action, key_mods );
	}

	void Application::OnMouseButtonEventInternal( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
	{
		// No need to query ImGui::GetIO().WantCaptureMouse here because Platform already queries it before calling this function.

		OnMouseButtonEvent( button, button_action, key_mods );
	}

	void Application::OnMouseScrollEventInternal( const float x_offset, const float y_offset )
	{
		// No need to query ImGui::GetIO().WantCaptureMouse here because Platform already queries it before calling this function.

		OnMouseScrollEvent( x_offset, y_offset );
	}

	void Application::OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels )
	{
		OnFramebufferResizeEvent( width_new_pixels, height_new_pixels );
	}

	void Application::CalculateTimeInformation()
	{
		ZoneScoped;

		time_since_start = Platform::CurrentTime();

		time_delta_real = time_since_start - time_previous_since_start;

		time_current += time_delta_real * time_multiplier;

		time_delta = time_current - time_previous;

		time_previous             = time_current;
		time_previous_since_start = time_since_start;

		time_sin      = Math::Sin( Radians( time_current ) );
		time_cos      = Math::Cos( Radians( time_current ) );
		time_mod_1    = std::fmod( time_current, 1.0f );
		time_mod_2_pi = std::fmod( time_current, Constants< float >::Two_Pi() );

		frame_count++;
	}

#ifdef _EDITOR
	void Application::RenderImGui()
	{
		ImGui::DockSpaceOverViewport();

		ImGuiDrawer::Update();

		RenderImGui_Viewport();
		RenderImGui_ViewportControls();

		if( ImGui::Begin( viewport_info.imgui_window_name.c_str() ) )
		{
			ImGui::Image( ( ImTextureID )renderer->FinalFramebuffer().ColorAttachment().Id().Get(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
		}

		ImGui::End();

		RenderImGui_FrameStatistics();

		if( show_gl_logger )
			gl_logger.Render( &show_gl_logger );

		scene_camera->RenderImGui( renderer->FinalFramebuffer().Size() );

		Engine::ImGuiDrawer::Draw( asset_database_texture.Assets(), { 400.0f, 512.0f } );

		renderer->RenderImGui();
	}

	void Application::RenderImGui_Viewport()
	{
		{
			const auto framebuffer_size = Platform::GetFramebufferSizeInPixels();
			ImGui::SetNextWindowSize( Math::CopyToImVec2( framebuffer_size ), ImGuiCond_FirstUseEver );
		}

		viewport_info.imgui_window_name = std::format( "Viewport {:d}x{:d}###Viewport", ( int )viewport_info.framebuffer_size.x, ( int )viewport_info.framebuffer_size.y );

		if( ImGui::Begin( viewport_info.imgui_window_name.c_str() ) )
		{
			viewport_info.framebuffer_size = ImGui::GetContentRegionAvail();
			const Vector2I viewport_available_size( ( int )viewport_info.framebuffer_size.x, ( int )viewport_info.framebuffer_size.y );

			const auto& imgui_io = ImGui::GetIO();
			if( viewport_available_size != renderer->FinalFramebuffer().Size() &&
				( not imgui_io.WantCaptureMouse || not imgui_io.MouseDown[ 0 ] ) )
			{
				OnFramebufferResizeEvent( viewport_available_size.X(), viewport_available_size.Y() );
			}

			/* Collect information for mouse hover/pos. info detection OUTSIDE the Begin()/End() block here. */
			if( viewport_info.is_hovered = ImGui::IsWindowHovered() )
			{
				/* If the Viewport is hovered, ImGui should NOT consume input and let the Application handle it: */
				ImGui::SetNextFrameWantCaptureMouse( false );
				ImGui::SetNextFrameWantCaptureKeyboard( false );
			}

			viewport_info.position_absolute = ImGui::GetCursorScreenPos();

			if( show_mouse_screen_space_position_overlay )
			{
				if( mouse_screen_space_position_overlay_is_active = IsMouseHoveringTheViewport() && ImGui::IsWindowFocused( ImGuiFocusedFlags_ChildWindows ) )
				{
					RenderImGui_CursorScreenSpacePositionOverlay();
					RenderImGui_MagnifierOverlay();
				}
			}

			/* ImGui::Image() call below is moved to a later point, to make sure the image itself stays the same until ImGui actually renders it. */
			//ImGui::Image( ( ImTextureID )renderer->FinalFramebuffer().ColorAttachment().Id().Get(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 });
		}

		ImGui::End();
	}

	void Application::RenderImGui_ViewportControls()
	{
		if( ImGuiUtility::BeginOverlay( viewport_info.imgui_window_name.c_str(), "##ViewportControls",
										ImGuiUtility::HorizontalPosition::LEFT, ImGuiUtility::VerticalPosition::TOP, 
										&show_frame_statistics_overlay ) )
		{
			int editor_shading_mode = ( int )renderer->GetEditorShadingMode();
			if( ImGuiUtility::DrawShadedSphereComboButton( "ShadingMode", reinterpret_cast< int* >( &editor_shading_mode ),
														   { 
																"Shaded",
																"Wireframe",
																"Shaded Wireframe", 
																"___",
																"Texture Coordinates",
																"Geometry Tangents",
																"Geometry Bitangents",
																"Geometry Normals", 
																"Debug Vectors",
																"Shading Normals"
														   } ) )
			{
				renderer->SetEditorShadingMode( ( EditorShadingMode )editor_shading_mode );
				const auto size = Platform::GetFramebufferSizeInPixels();
				Platform::ResizeWindow( size.X(), size.Y() ); // This is to prompt the Renderer to re-create the framebuffers and change the sRGBA status accordingly.
				ImGuiSetup::SetStyle( ( EditorShadingMode )editor_shading_mode == EditorShadingMode::Shaded || ( EditorShadingMode )editor_shading_mode == EditorShadingMode::ShadedWireframe );
			}
		}
		
		ImGuiUtility::EndOverlay();
	}

	void Application::RenderImGui_CursorScreenSpacePositionOverlay()
	{
		viewport_info.mouse_viewport_relative_position = Vector2I( GetMouseScreenSpacePosition() );
		const auto imgui_mouse_pos = ImGui::GetMousePos() + ImVec2( 5, -( ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().WindowPadding.y * 2 ) );

		if( Engine::ImGuiUtility::BeginOverlay( viewport_info.imgui_window_name.c_str(), "##Fragment Pos.", imgui_mouse_pos, &mouse_screen_space_position_overlay_is_active, false ) )
			ImGui::TextDisabled( "(%d, %d)", viewport_info.mouse_viewport_relative_position.X(), viewport_info.mouse_viewport_relative_position.Y() );

		Engine::ImGuiUtility::EndOverlay();
	}

	void Application::RenderImGui_MagnifierOverlay()
	{
		const float zoom = ( float )viewport_info.magnifier_zoom_factor;
		const float window_size = Math::Clamp( zoom * 32.0f, 256.0f, 512.0f );

		ASSERT( zoom >= 1.0f );

		const auto& style = ImGui::GetStyle();

		// Find out how many "original pixels" a magnifier pixel is:
		const float magnified_pixel_multiplier = 1.0f / zoom;

		// Calculate UV coordinates in the viewport texture:
		ImVec2 mouse_pos = ToImVec2( GetMouseScreenSpacePosition() + Vector2{ 0.5f, 0.5f } );
		ImVec2 uv_center = mouse_pos / viewport_info.framebuffer_size;
		ImVec2 uv_radius = ImVec2( 0.5f * window_size * magnified_pixel_multiplier,
								   0.5f * window_size * magnified_pixel_multiplier ) / viewport_info.framebuffer_size;

		ImVec2 uv0 = uv_center - uv_radius;
		ImVec2 uv1 = uv_center + uv_radius;

		// Clamp to [0,1] to avoid wrapping:
		uv0.x = Math::Clamp( uv0.x, 0.0f, 1.0f );
		uv0.y = Math::Clamp( uv0.y, 0.0f, 1.0f );
		uv1.x = Math::Clamp( uv1.x, 0.0f, 1.0f );
		uv1.y = Math::Clamp( uv1.y, 0.0f, 1.0f );

		std::swap( uv0.y, uv1.y );

		viewport_info.mouse_viewport_relative_position = Vector2I( GetMouseScreenSpacePosition() );
		const auto imgui_mouse_pos = ImGui::GetMousePos() + ImVec2( 5, 5 );

		if( ImGuiUtility::BeginOverlay( viewport_info.imgui_window_name.c_str(), "##Magnifier", imgui_mouse_pos, &mouse_screen_space_position_overlay_is_active, false ) )
		{
			local_persist GLuint nearest_sampler = 0;
			if( nearest_sampler == 0 ) // TODO: Put this inside its own class.
			{
				glGenSamplers( 1, &nearest_sampler );
				glSamplerParameteri( nearest_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
				glSamplerParameteri( nearest_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
			}

			const auto texture_id = renderer->FinalFramebuffer().ColorAttachment().Id().Get();
			glBindSampler( 0, nearest_sampler );
			const auto cursor_pos_before_image( ImGui::GetCursorScreenPos() );
			ImGui::Image( ( ImTextureID )texture_id, ImVec2( window_size, window_size ), uv0, uv1 );
			glBindSampler( 0, 0 );

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

			if( ImGuiUtility::BeginOverlay( viewport_info.imgui_window_name.c_str(),
											"##MagnifierLabel",
											imgui_mouse_pos + ImVec2( ( window_size - text_size.x ) * 0.5f, window_size + style.WindowPadding.y * 2.0f + 2 ),
											nullptr,
											false ) )
			{
				ImGui::TextUnformatted( zoom_label );
			}

			ImGuiUtility::EndOverlay();
		}
	}

	void Application::RenderImGui_FrameStatistics()
	{
		if( ImGuiUtility::BeginOverlay( viewport_info.imgui_window_name.c_str(), ICON_FA_CHART_LINE " Frame Statistics",
										ImGuiUtility::HorizontalPosition::RIGHT, ImGuiUtility::VerticalPosition::TOP,
										&show_frame_statistics_overlay ) )
		{
			const float fps        = 1.0f / time_delta_real;
			const float frame_time = time_delta_real * 1000.0f;

			constexpr std::uint8_t rolling_avg_fps_frame_count = 144;
			local_persist std::array< float, rolling_avg_fps_frame_count > last_N_fps_values = {};
			local_persist std::array< float, rolling_avg_fps_frame_count > last_N_frame_times = {};
			std::uint16_t rolling_avg_fps;
			float rolling_avg_frame_time;
			local_persist int rolling_avg_index = 0;

			/* Calculate rolling avg. fps & frame time: */
			{
				/* Since only 1 value in the ring buffer changes every frame, no need to re-calculate the total sum. Just add the difference between current value and
				 * the element at the current index in the ring buffer to the total sum. */
				local_persist float last_N_fps_values_sum = 0;
				local_persist float last_N_frame_times_sum = 0;

				const float previous_fps_at_this_index = last_N_fps_values[ rolling_avg_index ];
				last_N_fps_values_sum += fps - previous_fps_at_this_index;

				const float previous_frame_time_at_this_index = last_N_frame_times[ rolling_avg_index ];
				last_N_frame_times_sum += frame_time - previous_frame_time_at_this_index;

				last_N_fps_values[ rolling_avg_index ] = fps;
				last_N_frame_times[ rolling_avg_index++ ] = frame_time;

				rolling_avg_index = rolling_avg_index % rolling_avg_fps_frame_count;

				rolling_avg_fps        = ( std::uint16_t )Math::Round( last_N_fps_values_sum / rolling_avg_fps_frame_count );
				rolling_avg_frame_time = last_N_frame_times_sum / rolling_avg_fps_frame_count;
			}

			const auto& style = ImGui::GetStyle();

			const ImVec2 max_size( ImGui::CalcTextSize( "FPS: 999.9 fps  |  # Frames: 99999999" ) + style.ItemInnerSpacing );
			const ImVec2 max_size_half_width( max_size.x / 2.0f, max_size.y );
			const float  max_width = max_size.x;

			ImGui::SetWindowFontScale( 1.2f );
			char text[ 255 ] = {};
			sprintf_s( text,
					   "Avg. FPS:        %hu fps\n"
					   "Avg. Frame Time: %.2f ms\n"
					   "Time:            %.1f s\n"
					   "Frame:           #%-8lld",
					   rolling_avg_fps, rolling_avg_frame_time, time_since_start, frame_count );
			local_persist float refresh_rate = Platform::GetMainMonitorRefreshRate();
			ImGui::PushStyleColor( ImGuiCol_PlotLines, Math::ToImVec4( Math::Lerp( Color4::Red(), Color4::Green(), ( float )rolling_avg_fps / refresh_rate ) ) );
			ImGui::PlotLines( "##FPS", last_N_fps_values.data(), rolling_avg_fps_frame_count, rolling_avg_index, text,
							  rolling_avg_fps * 0.9f, rolling_avg_fps * 1.2f, ImVec2{ -1.0f, ImGui::GetTextLineHeight() * 6 } );
			ImGui::PopStyleColor();
			ImGui::SetWindowFontScale( 1.0f );

			/*if( not Math::IsEqual( time_multiplier, 1.0f ) )
			{
				ImGui::SameLine();
				ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_HeaderActive ), " (%.3f ms)", time_delta * 1000.0f );
			}*/

			ImGui::SetNextItemWidth( max_width - ImGui::CalcTextSize( "Time Multiplier" ).x );
			ImGui::SliderFloat( "Time Multiplier", &time_multiplier, 0.01f, 5.0f, "x %.2f", ImGuiSliderFlags_Logarithmic );

			if( !TimeIsFrozen() && ImGui::Button( ICON_FA_PAUSE " Pause", max_size_half_width ) )
				FreezeTime();
			else if( TimeIsFrozen() && ImGui::Button( ICON_FA_PLAY " Resume", max_size_half_width ) )
				UnfreezeTime();
			ImGui::SameLine();
			if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##time_multiplier", max_size_half_width ) )
				time_multiplier = 1.0f;
		}

		ImGuiUtility::EndOverlay();
	}
#endif // _EDITOR
}
