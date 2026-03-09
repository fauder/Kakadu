#pragma once

// std Includes.
#include <array>
#include <cstddef>

namespace Kakadu::Editor
{
	struct Command
	{
		enum class Type
		{
			Renderer_ToggleViewportRenderTarget,
			Renderer_HandlePendingViewportResize,
			Renderer_ChangeViewportShadingMode
		};

		Type type;

		alignas( std::max_align_t )
		std::array< std::uint8_t, 8 > payload;
	};
}
