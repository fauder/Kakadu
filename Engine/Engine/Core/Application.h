#pragma once

// Engine Includes.
#include "BitFlags.hpp"
#include "ImGuiLog.hpp"
#include "Math/Math.hpp"
#include "Platform.h"
#include "Graphics/GLLogger.h"

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
		Application( const BitFlags< CreationFlags >, const std::optional< int > msaa_sample_count = std::nullopt );
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

	private:
		void OnKeyboardEventInternal( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnFramebufferResizeEventInternal( const int width_new_pixels, const int height_new_pixels );

		void CalculateTimeInformation();

		void RenderImGui_FrameStatistics();
		std::uint16_t CalculateFPS_RollingAverage( const float fps_this_frame ) const;

	protected:
		bool display_frame_statistics;
		bool show_imgui;
		bool show_gl_logger;
		bool gamma_correction_is_enabled;

		float time_delta;
		float time_current;

		float time_multiplier;

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		long long frame_count;

		std::optional< int > msaa_sample_count;

		GLLogger gl_logger;

		bool vsync_is_enabled;

		/* 3 byte(s) of padding. */

	private:
		float time_delta_real;
		float time_previous;
		float time_previous_since_start;
		float time_since_start;
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication( const BitFlags< CreationFlags > );
}
