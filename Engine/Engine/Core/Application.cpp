#define IMGUI_DEFINE_MATH_PERATORS

// Engine Includes.
#include "Application.h"
#include "Graphics/Graphics.h"
#include "ImGuiDrawer.hpp"
#include "ImGuiSetup.h"
#include "ImGuiUtility.h"
#include "Graphics/Enums.h"
#include "Math/Math.hpp"
#include "Math/VectorConversion.hpp"

#ifdef _EDITOR
#include "Editor/ViewportScene.h"
#endif // _EDITOR

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>
#include <Tracy/tracy/Tracy.hpp>
#include <Tracy/tracy/TracyOpenGL.hpp>

/* To enable some .natvis functionality: */
#include "Natvis/Natvis.h"

// std Includes.
#include <iostream>

namespace Kakadu
{
	Application::Application( const BitFlags< CreationFlags > flags,
							  Renderer::Description&& renderer_description )
		:
		is_running( true ),
		gamma_correction_is_enabled( renderer_description.enable_gamma_correction && not flags.IsSet( CreationFlags::OnStart_DisableGammaCorrection ) ),
		vsync_is_enabled( false ),
		msaa_sample_count( renderer_description.main_framebuffer_msaa_sample_count )
	{
		NatVis::ForceIncludeInBuild();

		const auto begin = std::chrono::system_clock::now();

		// Application init.:
		{
			const auto begin = std::chrono::system_clock::now();
			Initialize();
			const auto end = std::chrono::system_clock::now();

			std::cout << "Engine-side Application Initialization:\n";
			std::cout << "  Application::Initialize(): " << std::chrono::duration_cast< std::chrono::milliseconds >( ( end - begin ) ).count() << " ms.\n";
		}

		// Renderer init.:
		{
			renderer_description.enable_gamma_correction = gamma_correction_is_enabled;
			const auto begin = std::chrono::system_clock::now();
			renderer = std::make_unique< Renderer >( std::move( renderer_description ) );
			const auto end = std::chrono::system_clock::now();
			std::cout << "  Renderer::Renderer(): " << std::chrono::duration_cast< std::chrono::milliseconds >( ( end - begin ) ).count() << " ms.\n";
		}

#ifdef _EDITOR
		// Editor context needs to be initialized after the Platform layer as scene camera etc. depend on framebuffer size etc.
		editor_context = std::make_unique< Editor::Context >( frame_time );

		editor_context->show_imgui                               = not flags.IsSet( CreationFlags::OnStart_DisableImGui );
		editor_context->show_imgui_demo_window                   = false;
		editor_context->show_frame_statistics_overlay            = true;
		editor_context->show_mouse_screen_space_position_overlay = false;
		editor_context->show_gl_logger                           = true;
		editor_context->ui_interaction_enabled                   = true;
#endif // _EDITOR

		const auto end = std::chrono::system_clock::now();
		std::cout << "    Engine-side total: " << std::chrono::duration_cast< std::chrono::milliseconds >( ( end - begin ) ).count() << " ms.\n";
	}

	Application::~Application()
	{
		Shutdown();
	}

	void Application::Run()
	{
#ifdef _EDITOR
		Platform::DestroySplashScreenAndSwitchToMainWindow();
#endif // _EDITOR

		TracyGpuContext;

		/* The main loop. */
		while( is_running )
		{
			TracyGpuZone( "GPU Time" );

			frame_time.Update();

			Platform::PollEvents();

			if( Platform::IsWindowIconified() )
			{
				Platform::Sleep( std::chrono::milliseconds( 200 ) );
				continue;
			}

			{
				ZoneScopedN( "Update" ); // Is (most probably) overridden in the client app. Makes sense to instrument here instead.
				Update();
			}

#ifdef _EDITOR

			/* Editor, when rendering the UI, will not (and can not) modify Application state directly; It enqueues commands instead. 
			 * These commands are processed and cleared in Application::Update() which is already executed for the frame.
			 * This means there is effectively a 1 frame delay in processing enqueued commands, which is fine and intended. */

			{
				ZoneScopedN( "Editor::Context::Update" );
				editor_context->Update( frame_time );
			}

			{
				ZoneScopedN( "Editor::RenderViewportScene" );
				Editor::RenderViewportScene( *renderer, editor_context->scene_camera.camera );
			}

			{
				ImGuiSetup::BeginFrame();
				ZoneScopedN( "Editor::Context::RenderUI" );
				editor_context->RenderUI( *renderer );
			}

			{
				ZoneScopedN( "RenderToolsUI" ); // Is overridden in the client app. Makes sense to instrument here instead.
				RenderToolsUI();
				ImGuiSetup::EndFrame();
			}
#else
			{
				ZoneScopedN( "RenderFrame" ); // Is (most probably) overridden in the client app. Makes sense to instrument here instead.
				RenderFrame();
			}
#endif // _EDITOR

			{
				ZoneScopedN( "Swap Buffers" );
				Platform::SwapBuffers();
			}

			TracyGpuCollect;
			FrameMark;

			is_running &= !Platform::ShouldClose();
		}
	}

	void Application::Initialize()
	{
		ServiceLocator< GLLogger >::Register( &gl_logger );
		ServiceLocator< AssetDatabase< Texture > >::Register( &asset_database_texture );
		ServiceLocator< AssetDatabase_Tracked< Texture* > >::Register( &asset_database_texture_tracked );
		ServiceLocator< AssetDatabase< Model > >::Register( &asset_database_model );
		ServiceLocator< MorphSystem >::Register( &morph_system );

		Platform::InitializeAndCreateWindows( vsync_is_enabled );

		const auto version = glGetString( GL_VERSION );
		std::cout << version << "\n\n";

		ImGuiSetup::Initialize( gamma_correction_is_enabled );
		ImGuiDrawer::Initialize();

		Platform::SetKeyboardEventCallback(
			[ = ]( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
			{
				this->HandleKeyboardEvent( key_code, key_action, key_mods );
			} );

		Platform::SetMouseButtonEventCallback(
			[ = ]( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
			{
				this->HandleMouseButtonEvent( button, button_action, key_mods );
			} );

		Platform::SetMouseScrollEventCallback(
			[ = ]( const float x_offset, const float y_offset )
			{
				this->HandleMouseScrollEvent( x_offset, y_offset );
			} );

		Platform::SetFramebufferResizeCallback(
			[ = ]( const int width_new_pixels, const int height_new_pixels )
			{
				this->HandleFramebufferResizeEvent( width_new_pixels, height_new_pixels );
			} );

#ifdef _EDITOR
		Platform::SetGLDebugOutputCallback( gl_logger.GetCallback() );
#endif // _EDITOR
	}

	void Application::Shutdown()
	{
		ImGuiSetup::Shutdown();

		Platform::Shutdown();
	}

	void Application::Update()
	{
#ifdef _EDITOR
		ProcessEditorCommands();
#endif // _EDITOR

		morph_system.Execute( frame_time.time_delta, frame_time.time_delta_real );

		renderer->Update();
	}

	void Application::RenderFrame()
	{
		// Client App can override this to inject custom rendering code.
		// TODO: Implement actual game camera rendering.
	}

	void Application::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
#ifdef _EDITOR
		editor_context->OnKeyboardEvent( key_code, key_action, key_mods );
#endif // _EDITOR
	}

	void Application::OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
	{
#ifdef _EDITOR
		editor_context->OnMouseButtonEvent( button, button_action, key_mods );
#endif // _EDITOR
	}

	void Application::OnMouseScrollEvent( const float x_offset, const float y_offset )
	{
#ifdef _EDITOR
		editor_context->OnMouseScrollEvent( x_offset, y_offset );
#endif // _EDITOR
	}

	void Application::HandleKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
	{
		// No need to query ImGui::GetIO().WantCaptureKeyboard here because Platform already queries it before calling this function.

		OnKeyboardEvent( key_code, key_action, key_mods );
	}

	void Application::HandleMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
	{
		// No need to query ImGui::GetIO().WantCaptureMouse here because Platform already queries it before calling this function.

		OnMouseButtonEvent( button, button_action, key_mods );
	}

	void Application::HandleMouseScrollEvent( const float x_offset, const float y_offset )
	{
		// No need to query ImGui::GetIO().WantCaptureMouse here because Platform already queries it before calling this function.

		OnMouseScrollEvent( x_offset, y_offset );
	}

	void Application::HandleFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
	{
		/* Do nothing on minimize: */
		if( width_new_pixels <= 0 || height_new_pixels <= 0 ||
			( renderer->EditorViewportFramebuffer().Size() == Vector2I{ width_new_pixels, height_new_pixels } ) )
			return;

		renderer->OnFramebufferResize( width_new_pixels, height_new_pixels );

#ifdef _EDITOR
		editor_context->OnFramebufferResizeEvent( width_new_pixels, height_new_pixels );
#endif // _EDITOR

		OnFramebufferResizeEvent( width_new_pixels, height_new_pixels );
	}

#ifdef _EDITOR
	void Application::ProcessEditorCommands()
	{
		while( not editor_context->commands_queue.empty() )
		{
			auto& command = editor_context->commands_queue.front();
			switch( command.type )
			{
				case Editor::Command::Type::Renderer_HandlePendingViewportResize:
				{
					const Vector2I framebuffer_size = std::bit_cast< Vector2I >( command.payload );
					HandleFramebufferResizeEvent( framebuffer_size.X(), framebuffer_size.Y() );
					break;
				}
				case Editor::Command::Type::Renderer_ToggleViewportRenderTarget:
				{
					if( editor_context->show_imgui )
					{
						renderer->SetFinalOutputToEditorViewport();
					}
					else
					{
						const Vector2I framebuffer_size = Platform::GetFramebufferSizeInPixels();
						HandleFramebufferResizeEvent( framebuffer_size.X(), framebuffer_size.Y() );
						renderer->SetFinalOutputToDefaultFramebuffer();
					}
					break;
				}
				case Editor::Command::Type::Renderer_ChangeEditorShadingMode:
				{
					int option;
					std::memcpy( &option, command.payload.data(), sizeof( option ) );

					renderer->SetEditorShadingMode( ( EditorShadingMode )option );
					const auto size = Platform::GetFramebufferSizeInPixels();
					Platform::ResizeWindow( size.X(), size.Y() ); // This is to prompt the Renderer to re-create the framebuffers and change the sRGBA status accordingly.
					ImGuiSetup::SetStyle( ( EditorShadingMode )option == EditorShadingMode::Shaded ||
										  ( EditorShadingMode )option == EditorShadingMode::ShadedWireframe );

					break;
				}
				default:
					break;
			}

			editor_context->commands_queue.pop();
		}
	}
#endif // _EDITOR
}
