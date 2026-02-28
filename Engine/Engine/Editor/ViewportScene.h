#pragma once

/* Forward Declarations: */
namespace Engine
{
	class Renderer;
	class Camera;
}

namespace Engine::Editor
{
	void RenderViewportScene( Renderer& renderer, Camera& scene_camera );
}
