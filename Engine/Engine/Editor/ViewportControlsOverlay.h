#pragma once

// Engine Includes.
#include "EditorContext.h"

/* Forward Declarations: */
namespace Engine
{
	class Renderer;
}

namespace Engine::Editor
{
	void RenderViewportControlsOverlay( Context& editor_context, const Renderer& renderer );
}
