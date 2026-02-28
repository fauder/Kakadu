#pragma once

// Engine Includes.
#include "EditorContext.h"

/* Forward Declarations: */
namespace Kakadu
{
	class Renderer;
}

namespace Kakadu::Editor
{
	void RenderViewportControlsOverlay( Context& editor_context, const Renderer& renderer );
}
