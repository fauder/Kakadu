#pragma once

// Engine Includes.
#include "AssetDatabase.hpp"
#include "AssetDatabase_Tracked.hpp"
#include "BitFlags.hpp"
#include "MorphSystem.h"
#include "Platform.h"
#include "FrameTime.h"
#include "Math/Math.hpp"
#include "Graphics/Model.h"
#include "Graphics/Renderer.h"
#include "Graphics/RHI/Texture.h"

// std Includes.
#include <functional>

#ifdef _EDITOR
#include "Editor/EditorContext.h"
#endif // _EDITOR

namespace Kakadu
{
	enum class CreationFlags
	{
		None                 = 0,
		OnStart_DisableImGui = 1
	};

	struct ApplicationCallbacks
	{
		std::function< void() > on_initialize;
		std::function< void() > on_shutdown;
		std::function< void() > on_update;
		std::function< void() > on_render_frame;    // This will be standalone only.
		std::function< void() > on_render_tools_ui; // This will be editor only.

		std::function< void( Platform::KeyCode, Platform::KeyAction, Platform::KeyMods ) >				on_keyboard_event;
		std::function< void( Platform::MouseButton, Platform::MouseButtonAction, Platform::KeyMods ) >	on_mouse_button_event;
		std::function< void( float, float ) >															on_mouse_scroll_event;
		std::function< void( i32, i32 ) >																on_framebuffer_resize;
	};

	class Application
	{
	public:
		Application( ApplicationCallbacks&&,
					 const BitFlags< CreationFlags > flags,
					 Renderer::Description&& renderer_description );
		~Application();

		void Run();

	protected:
		void Quit() { is_running = false; }

		void FreezeTime()   { frame_time.FreezeTime(); }
		void UnfreezeTime() { frame_time.UnfreezeTime(); }
		bool TimeIsFrozen() { return frame_time.TimeIsFrozen(); }

		bool MSAAIsEnabled() { return msaa_sample_count.has_value(); }

	private:
		void Initialize();
		void Update();
		void Shutdown();

		/*
		 * Application-internal event handlers; These do processing and then relay the events to client applications.
		 */

		void HandleKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void HandleMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		void HandleMouseScrollEvent( const float x_offset, const float y_offset );
		void HandleFramebufferResizeEvent( const i32 width_new_pixels, const i32 height_new_pixels );

#ifdef _EDITOR
		void ProcessEditorCommands();
#endif // _EDITOR

	protected:

#ifdef _EDITOR
		std::unique_ptr< Editor::Context > editor_context;
#endif // _EDITOR

		AssetDatabase< RHI::Texture > asset_database_texture;
		AssetDatabase_Tracked< RHI::Texture* > asset_database_texture_tracked;
		AssetDatabase< Model > asset_database_model;

		std::unique_ptr< Renderer > renderer;

		std::optional< i32 > msaa_sample_count;

		MorphSystem morph_system;

		FrameTime frame_time;

		bool is_running;

		bool vsync_is_enabled;

		/* 6 byte(s) of padding. */

	private:
		ApplicationCallbacks callbacks;
	};
}
