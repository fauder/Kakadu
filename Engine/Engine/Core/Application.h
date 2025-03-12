#pragma once

// Engine Includes.
#include "BitFlags.hpp"
#include "ImGuiLog.hpp"
#include "Math/Math.hpp"
#include "Platform.h"
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
		virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels );
		virtual void RenderImGui();

	protected:
		inline void FreezeTime()   { time_multiplier = 0.0f; }
		inline void UnfreezeTime() { time_multiplier = 1.0f; }
		inline bool TimeIsFrozen() { return Math::IsZero( time_multiplier ); }

		inline bool MSAAIsEnabled() { return msaa_sample_count.has_value(); }

		void SetImGuiViewportImageID( const unsigned int id );

	private:
		void OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels );

		void CalculateTimeInformation();

		void RenderImGui_FrameStatistics();
		void RenderImGui_Viewport( const unsigned int texture_id );
		std::uint16_t CalculateFPS_RollingAverage( const float fps_this_frame ) const;

	protected:
		std::unique_ptr< Renderer > renderer;

		bool show_frame_statistics;
		bool show_imgui;
		bool show_gl_logger;

		bool gamma_correction_is_enabled;
		bool vsync_is_enabled;

		/* 3 byte(s) of padding. */

		float time_delta;
		float time_current;

		float time_multiplier;

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		/* 4 byte(s) of padding. */

		long long frame_count;

		std::optional< int > msaa_sample_count;

		GLLogger gl_logger;

	private:
		float time_delta_real;
		float time_previous;
		float time_previous_since_start;
		float time_since_start;

		unsigned int imgui_viewport_texture_id;

		/* 4 byte(s) of padding. */
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication( const BitFlags< CreationFlags > );
}
