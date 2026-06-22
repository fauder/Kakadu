#pragma once

/* Forward Declarations: */
namespace Kakadu
{
	class Renderer;
	class Camera;
}

namespace Kakadu::Editor
{
	void RenderViewportScene( Renderer& renderer, Camera& scene_camera );
}
