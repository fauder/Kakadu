// Engine Includes.
#include "ImGuiCustomColors.h"

namespace Engine::ImGuiCustomColors
{
	void Text( const char* text, const CustomColorType type )
	{
		PushStyleColor( ImGuiCol_Text, type );
		ImGui::TextUnformatted( text );
		ImGui::PopStyleColor();
	}

	void PushStyleColor( ImGuiCol idx, const CustomColorType type )
	{
		switch( type )
		{
			case CustomColorType::HDR:		ImGui::PushStyleColor( ImGuiCol_Text, COLOR_HDR );		break;
			case CustomColorType::MSAA:		ImGui::PushStyleColor( ImGuiCol_Text, COLOR_MSAA );		break;
		}
	}
}