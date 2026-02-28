#pragma once

// Engine Includes.
#include "AssetDatabase.hpp"
#include "AssetDatabase_Tracked.hpp"
#include "BitFlags.hpp"
#include "ImGuiLog.hpp"
#include "MorphSystem.h"
#include "Platform.h"
#include "FrameTime.h"
#include "Math/Math.hpp"
#include "Graphics/GLLogger.h"
#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Graphics/Texture.h"

#ifdef _EDITOR
#include "Editor/EditorContext.h" 
#endif // _EDITOR

namespace Kakadu
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

		virtual void RenderToolsUI() = 0;

		virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		virtual void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		virtual void OnMouseScrollEvent( const float x_offset, const float y_offset );
		virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) = 0; // Application calls the internal one => this one is pure virtual.
		
		void Quit() { is_running = false; }

		void FreezeTime()   { frame_time.FreezeTime(); }
		void UnfreezeTime() { frame_time.UnfreezeTime(); }
		bool TimeIsFrozen() { return frame_time.TimeIsFrozen(); }

		bool MSAAIsEnabled() { return msaa_sample_count.has_value(); }

	private:
		/*
		 * Application-internal event handlers; These do processing and then relay the events to client applications.
		 */

		void HandleKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void HandleMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		void HandleMouseScrollEvent( const float x_offset, const float y_offset  );
		void HandleFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels );

#ifdef _EDITOR
		void ProcessEditorCommands();
#endif // _EDITOR

	protected:

#ifdef _EDITOR
		std::unique_ptr< Editor::Context > editor_context;

		GLLogger gl_logger;
#endif // _EDITOR

		AssetDatabase< Texture > asset_database_texture;
		AssetDatabase_Tracked< Texture* > asset_database_texture_tracked;
		AssetDatabase< Model > asset_database_model;

		std::unique_ptr< Renderer > renderer;

		std::optional< int > msaa_sample_count;

		MorphSystem morph_system;

		FrameTime frame_time;

		bool is_running;

		bool gamma_correction_is_enabled;
		bool vsync_is_enabled;

		/* 5 byte(s) of padding. */
	};

	/* Needs to be implemented by the CLIENT Application. */
	Application* CreateApplication( const BitFlags< CreationFlags > );
}
