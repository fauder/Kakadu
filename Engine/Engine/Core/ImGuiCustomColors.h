#pragma once

// Vendor Includes.
#include <ImGui/imgui.h>

// std Includes.
#include <cstdint>

namespace Engine::ImGuiCustomColors
{
	constexpr ImVec4 COLOR_HDR( 0.87f, 0.75f, 0.35f, 1.0f );
	constexpr ImVec4 COLOR_MSAA( 0.6f, 0.34f, 1.0f, 1.0f );

	enum class CustomColorType : std::uint8_t
	{
		HDR,
		MSAA
	};

	void Text( const char* text, const CustomColorType type );
	void PushStyleColor( ImGuiCol idx, const CustomColorType type );
}
