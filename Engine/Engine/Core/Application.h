#pragma once

// Engine Includes.
#include "BitFlags.hpp"
#include "ImGuiLog.hpp"
#include "MorphSystem.h"
#include "Platform.h"
#include "Math/Math.hpp"
#include "Graphics/GLLogger.h"
#include "Graphics/Renderer.h"

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

		virtual void Initialize();
		virtual void Shutdown();

		virtual void Run();

		virtual void Update();

		virtual void Render();

		virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		virtual void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		virtual void OnMouseScrollEvent( const float x_offset, const float y_offset );
		virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels );
		void OnFramebufferResizeEvent( const Vector2I new_size_pixels ); // Convenience overload.
		
#ifdef _EDITOR
		virtual void RenderImGui();
#endif // _EDITOR

	protected:
		struct ViewportWindowInfo
		{
			static constexpr std::uint8_t SMALLEST_MAGNIFIER_ZOOM_FACTOR = 4;
			static constexpr std::uint8_t LARGEST_MAGNIFIER_ZOOM_FACTOR  = 32;

			ImVec2 framebuffer_size; // The OpenGL framebuffer size.
			ImVec2 position_absolute; // The ImGui window position_absolute.
			Vector2I mouse_viewport_relative_position; // Screen-space position of the mouse, relative to OpenGL convention: the bottom-left of the viewport.
			std::uint8_t magnifier_zoom_factor = SMALLEST_MAGNIFIER_ZOOM_FACTOR;
			bool is_hovered = false;
			/* 3 bytes(s) of padding. */
		};

		inline void FreezeTime()   { time_multiplier = 0.0f; }
		inline void UnfreezeTime() { time_multiplier = 1.0f; }
		inline bool TimeIsFrozen() { return Math::IsZero( time_multiplier ); }

		inline bool MSAAIsEnabled() { return msaa_sample_count.has_value(); }

#ifdef _EDITOR
		bool IsMouseHoveringTheViewport() const { return viewport_info.is_hovered; }
		/* This returns the viewport coordinates (bottom-left origin for OpenGL). Beware: may return garbage when the mouse is outside the viewport. */
		Vector2 GetMouseScreenSpacePosition() const;
		void SetViewportMagnifierZoomFactor( const std::uint8_t new_zoom_factor );
		void OffsetViewportMagnifierZoomFactor( const bool increment );
#endif

	private:
		void OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnMouseButtonEventInternal( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		void OnMouseScrollEventInternal( const float x_offset, const float y_offset  );
		void OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels );

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

		/* 3 bytes(s) of padding. */

		ViewportWindowInfo viewport_info;

		/* 4 bytes(s) of padding. */
#endif // _EDITOR

		std::unique_ptr< Renderer > renderer;

		bool gamma_correction_is_enabled;
		bool vsync_is_enabled;

		/* 2 byte(s) of padding. */

		float time_delta;
		float time_current;

		float time_multiplier;

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		long long frame_count;

		std::optional< int > msaa_sample_count;

	private:
		MorphSystem morph_system;

		float time_delta_real;
		float time_previous;
		float time_previous_since_start;
		float time_since_start;

		bool mouse_screen_space_position_overlay_is_active;

		/* 7 byte(s) of padding. */
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication( const BitFlags< CreationFlags > );
}
