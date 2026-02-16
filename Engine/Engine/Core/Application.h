#pragma once

// Engine Includes.
#include "AssetDatabase.hpp"
#include "AssetDatabase_Tracked.hpp"
#include "BitFlags.hpp"
#include "ImGuiLog.hpp"
#include "MorphSystem.h"
#include "Platform.h"
#include "Math/Math.hpp"
#include "Graphics/GLLogger.h"
#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"

#ifdef _EDITOR
#include "Editor/SceneCamera.h" 
#endif // _EDITOR

namespace Engine
{
	enum class CreationFlags
	{
		None                           = 0,
		OnStart_DisableImGui           = 1,
		OnStart_DisableGammaCorrection = 2,
	};

	class Application
	{
	public:
		Application( const BitFlags< CreationFlags > flags,
					 Renderer::Description&& renderer_description );
		virtual ~Application();

		void Run();

	protected:
		virtual void Initialize();
		virtual void Shutdown();

		virtual void Update();

		virtual void RenderFrame();
#ifdef _EDITOR
		void RenderViewport();
#endif

		virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		virtual void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		virtual void OnMouseScrollEvent( const float x_offset, const float y_offset );
		virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) = 0; // Application calls the internal already => this one is pure virtual.
		
#ifdef _EDITOR
		virtual void RenderImGui();
#endif // _EDITOR

		struct ViewportWindowInfo
		{
			static constexpr std::uint8_t SMALLEST_MAGNIFIER_ZOOM_FACTOR = 2;
			static constexpr std::uint8_t LARGEST_MAGNIFIER_ZOOM_FACTOR  = 32;

			std::string imgui_window_name;
			ImVec2 framebuffer_size; // The OpenGL framebuffer size.
			ImVec2 position_absolute; // The ImGui window position_absolute.
			Vector2I mouse_viewport_relative_position; // Screen-space position of the mouse, relative to OpenGL convention: the bottom-left of the viewport.
			std::uint8_t magnifier_zoom_factor = SMALLEST_MAGNIFIER_ZOOM_FACTOR;
			bool is_hovered = false;
			/* 3 bytes(s) of padding. */
		};

		void Quit() { is_running = false; }

		void FreezeTime()   { time_multiplier = 0.0f; }
		void UnfreezeTime() { time_multiplier = 1.0f; }
		bool TimeIsFrozen() { return Math::IsZero( time_multiplier ); }

		bool MSAAIsEnabled() { return msaa_sample_count.has_value(); }

#ifdef _EDITOR
		bool IsMouseHoveringTheViewport() const { return viewport_info.is_hovered; }
		/* This returns the viewport coordinates (bottom-left origin for OpenGL). Beware: may return garbage when the mouse is outside the viewport. */
		Vector2 GetMouseScreenSpacePosition() const;
		void SetViewportMagnifierZoomFactor( const std::uint8_t new_zoom_factor );
		void OffsetViewportMagnifierZoomFactor( const bool increment );
#endif

	private:
		/*
		 * Application-internal event handlers; These do processing and then relay the events to client applications.
		 */

		void HandleKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void HandleMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		void HandleMouseScrollEvent( const float x_offset, const float y_offset  );
		void HandleFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels );

		void CalculateTimeInformation();

#ifdef _EDITOR
		void RenderImGui_Viewport();
		void RenderImGui_ViewportControls();
		void RenderImGui_CursorScreenSpacePositionOverlay();
		void RenderImGui_MagnifierOverlay();
		void RenderImGui_FrameStatistics();
#endif // _EDITOR

	protected:
#ifdef _EDITOR
		GLLogger gl_logger;

		bool show_frame_statistics_overlay;
		bool show_mouse_screen_space_position_overlay;
		bool show_imgui;
		bool show_imgui_demo_window;
		bool show_gl_logger;

		bool ui_interaction_enabled;
		/* 2 bytes(s) of padding. */

		ViewportWindowInfo viewport_info;

		/* 
		 * Scene Camera:
		 */

		// Needs to be initialized after the Platform layer is initialized to be able to query framebuffer size for aspect ratio => hence the unique_ptr.
		std::unique_ptr< Editor::SceneCamera > scene_camera;

#endif // _EDITOR

		AssetDatabase< Texture > asset_database_texture;
		AssetDatabase_Tracked< Texture* > asset_database_texture_tracked;
		AssetDatabase< Model > asset_database_model;

		std::unique_ptr< Renderer > renderer;

		bool is_running;

		bool gamma_correction_is_enabled;
		bool vsync_is_enabled;

		/* 1 byte(s) of padding. */

		float time_delta;
		float time_current;

		float time_multiplier;

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		long long frame_count;

		std::optional< int > msaa_sample_count;

		MorphSystem morph_system;

	private:
		float time_delta_real;
		float time_previous;
		float time_previous_since_start;
		float time_since_start;

		bool mouse_screen_space_position_overlay_is_active;

		/* 1 byte(s) of padding. */

		std::uint16_t rolling_avg_fps;
		int rolling_avg_index;

		float rolling_avg_frame_time;

		static constexpr std::uint8_t ROLLING_AVG_FPS_FRAME_COUNT = 144;
		std::array< float, ROLLING_AVG_FPS_FRAME_COUNT > last_N_fps_values;
		std::array< float, ROLLING_AVG_FPS_FRAME_COUNT > last_N_frame_times;
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication( const BitFlags< CreationFlags > );
}
