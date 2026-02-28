// Engine Includes.
#include "ViewportScene.h"
#include "Graphics/Renderer.h"

namespace Engine::Editor
{
	void RenderViewportScene( Renderer& renderer, Camera& scene_camera )
	{
		renderer.UpdatePerPass( Engine::Renderer::RENDER_PASS_ID_LIGHTING, scene_camera );
		renderer.RenderFrame();
	}
}