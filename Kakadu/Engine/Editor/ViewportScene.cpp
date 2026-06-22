// Engine Includes.
#include "ViewportScene.h"
#include "Graphics/Renderer.h"
#include "Graphics/RHI/GLDebugGroup.h" // TODO: Enable only for non-standalone builds.

namespace Kakadu::Editor
{
	void RenderViewportScene( Renderer& renderer, Camera& scene_camera )
	{
		KAKADU_GL_DEBUG_GROUP( "Editor::RenderViewportScene()" );

		renderer.UpdatePerPass( Kakadu::Renderer::RENDER_PASS_ID_LIGHTING, scene_camera );
		renderer.RenderFrame();
	}
}