#pragma once

// Engine Includes.
#include "Core/Types.h"

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
		std::array< u8, 8 > payload;
	};
}
