#pragma once

// Engine Includes.
#include "Introspection/RendererIntrospectionSurface.h"
#include "Math/Color.hpp"

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
