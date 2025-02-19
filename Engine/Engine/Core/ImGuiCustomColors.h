#pragma once

// Vendor Includes.
#include <ImGui/imgui.h>

// std Includes.
#include <cstdint>

namespace Engine::ImGuiCustomColors
{
	constexpr ImVec4 COLOR_HDR( 0.96f, 0.76f, 0.04f, 1.0f );
	constexpr ImVec4 COLOR_MSAA( 0.36f, 0.18f, 0.57f, 1.0f );

	enum class CustomColorType : std::uint8_t
	{
		HDR,
		MSAA
	};

	void Text( const char* text, const CustomColorType type );
	void PushStyleColor( ImGuiCol idx, const CustomColorType type );
}
