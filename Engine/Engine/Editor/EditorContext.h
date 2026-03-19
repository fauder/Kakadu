#pragma once

// Engine Includes.
#include "EditorCommand.h"
#include "SceneCamera.h"
#include "ViewportPanel.h"
#include "Core/Platform.h"
#include "Core/FrameTime.h"
#include "Introspection/RendererIntrospectionSurface.h"
#include "Math/Vector.hpp"

// std Includes.
#include <queue>

/* Forward Declarations: */
namespace Kakadu
{
	class Renderer;
}

namespace Kakadu::Editor
{
	struct Context
	{
		void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods );
		void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods );
		void OnMouseScrollEvent( const float x_offset, const float y_offset );

		void OnFramebufferResizeEvent( const i32 width_new_pixels, const i32 height_new_pixels );

		void Initialize();
		void Update( const FrameTime& frame_time );
		void RenderUI();

		FrameTime& frame_time;
		Renderer* renderer;

		SceneCamera scene_camera;

		Vector2I viewport_resolution;

		ViewportPanel viewport_panel;

		RendererIntrospectionSurface renderer_introspection_surface;

		std::queue< Command > commands_queue;

		bool show_imgui;
		bool show_imgui_demo_window;
		bool show_frame_statistics_overlay;
		bool show_mouse_screen_space_position_overlay;
		bool show_gl_logger;

		bool mouse_screen_space_position_overlay_is_active; // TODO: Code smell; Why 2 bools? Get rid of this.

		bool ui_interaction_enabled; // TODO: Replace this with proper viewport input handling like Unity etc.
		/* 5 bytes(s) of padding. */
	};
};
