// Engine Includes.
#include "ViewportScene.h"
#include "Graphics/Renderer.h"

namespace Kakadu::Editor
{
	void RenderViewportScene( Renderer& renderer, Camera& scene_camera )
	{
		renderer.UpdatePerPass( Kakadu::Renderer::RENDER_PASS_ID_LIGHTING, scene_camera );
		renderer.RenderFrame();
	}
}