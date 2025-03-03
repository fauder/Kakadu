#define IMGUI_DEFINE_MATH_OPERATORS

// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiDrawer.hpp"
#include "ImGuiSetup.h"
#include "ImGuiUtility.h"
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
	Application::Application( const BitFlags< CreationFlags > flags, const std::optional< int > msaa_sample_count )
		:
		show_frame_statistics( true ),
		show_imgui( not flags.IsSet( CreationFlags::OnStart_DisableImGui ) ),
		show_gl_logger( true ),
		gamma_correction_is_enabled( not flags.IsSet( CreationFlags::OnStart_DisableGammaCorrection ) ),
		msaa_sample_count( msaa_sample_count ),
		time_current( 0.0f ),
		time_multiplier( 1.0f ),
		time_previous( 0.0f ),
		time_previous_since_start( 0.0f ),
		time_since_start( 0.0f ),
		frame_count( 1 ),
		vsync_is_enabled( false )
	{
		NatVis::ForceIncludeInBuild();

		Initialize();
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Initialize()
	{
		Engine::ServiceLocator< Engine::GLLogger >::Register( &gl_logger );

		Platform::InitializeAndCreateWindow( 800, 600, msaa_sample_count, vsync_is_enabled );

		const auto version = glGetString( GL_VERSION );
		std::cout << version << "\n\n";

		ImGuiSetup::Initialize( gamma_correction_is_enabled );
		ImGuiDrawer::Initialize();

		Platform::SetKeyboardEventCallback(
			[ = ]( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
			{
				this->OnKeyboardEventInternal( key_code, key_action, key_mods );
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

			{
				ZoneScopedN( "Update" ); // Is (most probably) overridden in the client app. Makes sense to instrument here instead.
				Update();
			}

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
	}

	void Application::Render()
	{
	}

	void Application::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		switch( key_code )
		{
			case Platform::KeyCode::KEY_ESCAPE:
				/*if( key_action == Platform::KeyAction::PRESS )
					Platform::SetShouldClose( true );*/
				break;
			default:
				break;
		}
	}

	void Application::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
	{
	}

	void Application::SetImGuiViewportImageID( const unsigned int id )
	{
		imgui_viewport_texture_id = id;
	}

	void Application::OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return;

		OnKeyboardEvent( key_code, key_action, key_mods );
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

		time_previous = time_current;
		time_previous_since_start = time_since_start;

		time_sin = Math::Sin( Radians( time_current ) );
		time_cos = Math::Cos( Radians( time_current ) );
		time_mod_1 = std::fmod( time_current, 1.0f );
		time_mod_2_pi = std::fmod( time_current, Constants< float >::Two_Pi() );

		frame_count++;
	}

	void Application::RenderImGui()
	{
		ImGui::DockSpaceOverViewport();

		ImGuiDrawer::Update();

		RenderImGui_Viewport( imgui_viewport_texture_id );
		RenderImGui_FrameStatistics();

		if( show_gl_logger )
			gl_logger.Render( &show_gl_logger );
	}

	void Application::RenderImGui_FrameStatistics()
	{
		if( ImGuiUtility::BeginOverlay( "Viewport", ICON_FA_CHART_LINE " Frame Statistics", &show_frame_statistics ) )
		{
			const auto fps = 1.0f / time_delta_real;

			const std::uint16_t rolling_average_fps = CalculateFPS_RollingAverage( fps );

			const auto& style = ImGui::GetStyle();

			const ImVec2 max_size( ImGui::CalcTextSize( "FPS: 999.9 fps  |  # Frames: 99999999" ) + style.ItemInnerSpacing );
			const ImVec2 max_size_half_width( max_size.x / 2.0f, max_size.y );
			const float  max_width = max_size.x;

			ImGui::Text( "FPS (Avg.): %hu fps", rolling_average_fps );
			ImGui::Text( "Time: %.1f s.  |  Frame #%lu", time_since_start, frame_count );
			ImGui::Text( "Frame Time: %.1f ms", time_delta_real * 1000.0f );
			if( not Math::IsEqual( time_multiplier, 1.0f ) )
			{
				ImGui::SameLine();
				ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_HeaderActive ), " (%.3f ms)", time_delta * 1000.0f );
			}

			// TODO: Show immediate (float) fps in the histogram plot.

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

	void Application::RenderImGui_Viewport( const unsigned int texture_id )
	{
		{
			const auto framebuffer_size = Platform::GetFramebufferSizeInPixels();
			ImGui::SetNextWindowSize( Engine::Math::CopyToImVec2( framebuffer_size ), ImGuiCond_Appearing );
		}

		if( ImGui::Begin( "Viewport" ) )
		{
			const ImVec2   viewport_size_imvec2( ImGui::GetContentRegionAvail() );
			const Vector2I viewport_size( ( int )viewport_size_imvec2.x, ( int )viewport_size_imvec2.y );

			const auto& imgui_io = ImGui::GetIO();
			if( ( imgui_io.WantCaptureMouse && imgui_io.MouseReleased[ 0 ] ) ||
				( not imgui_io.WantCaptureMouse && Platform::IsMouseButtonReleased( Platform::MouseButton::Left ) ) )
				OnFramebufferResizeEvent( viewport_size.X(), viewport_size.Y() );

			if( ImGui::IsWindowHovered() )
			{
				ImGui::SetNextFrameWantCaptureMouse( false );
				ImGui::SetNextFrameWantCaptureKeyboard( false );
			}

			ImGui::Image( ( void* )( intptr_t )texture_id, ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
		}

		ImGui::End();
	}

	std::uint16_t Application::CalculateFPS_RollingAverage( const float fps_this_frame ) const
	{
		static int index = 0;
		constexpr auto N = 144;

		static std::array< float, N > last_N_fps_values = {};

		/* Since only 1 value in the ring buffer changes every frame, no need to re-calculate the total sum. Just add the difference between current fps and
		 * the element at the current index in the framebuffer to the total sum. */
		static float last_N_fps_values_sum = 0;

		const float previous_fps_at_this_index = last_N_fps_values[ index ];
		last_N_fps_values_sum += fps_this_frame - previous_fps_at_this_index;

		last_N_fps_values[ index++ ] = fps_this_frame;
		index = index % N;

		return ( std::uint16_t )Math::Round( last_N_fps_values_sum / N );
	}
}
