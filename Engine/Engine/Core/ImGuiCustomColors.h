#pragma once

// Vendor Includes.
#include <ImGui/imgui.h>

// std Includes.
#include <cstdint>

namespace Engine::ImGuiCustomColors
{
	constexpr ImU32 COLOR_HDR  = IM_COL32( 0.96f, 0.76f, 0.04f, 1.0f );
	constexpr ImU32 COLOR_MSAA = IM_COL32( 0.36f, 0.18f, 0.57f, 1.0f );

	enum class CustomColorType : std::uint8_t
	{
		HDR,
		MSAA
	};

	void Text( const char* text, const CustomColorType type );
	void PushStyleColor( ImGuiCol idx, const CustomColorType type );
}
