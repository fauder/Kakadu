#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Introspection/RendererIntrospectionSurface.h"

namespace Kakadu
{
	/* Forward Declarations: */
	class Renderer;

	namespace Editor
	{
		struct RendererPanel
		{
			void Render( Renderer& renderer, RendererIntrospectionSurface& introspection_surface );
		};
	}
}
