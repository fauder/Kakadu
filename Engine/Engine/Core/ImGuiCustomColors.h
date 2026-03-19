#pragma once

// Engine Includes.
#include "Types.h"

// Vendor Includes.
#include <ImGui/imgui.h>

// std Includes.
#include <cstdint>

namespace Kakadu::ImGuiCustomColors
{
	constexpr ImU32 COLOR_HDR  = IM_COL32( 245, 194, 10, 255 );
	constexpr ImU32 COLOR_MSAA = IM_COL32( 92, 46, 145, 255 );

	enum class CustomColorType : u8
	{
		HDR,
		MSAA
	};

	void Text( const char* text, const CustomColorType type );
	void PushStyleColor( ImGuiCol idx, const CustomColorType type );
}
