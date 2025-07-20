#pragma once

// std Includes.
#include <array>

// Vendor Includes.
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui.h>

namespace Engine::ImGuiUtility
{
	enum class HorizontalPosition	{ LEFT, CENTER, RIGHT };
	enum class VerticalPosition		{ TOP, CENTER, BOTTOM };

	void Table_Header_ManuallySubmit( const int column_index );
	template< int array_size >
	void Table_Header_ManuallySubmit( const std::array< int, array_size >& column_indices )
	{
		for( std::size_t i = 0; i < array_size; i++ )
			Table_Header_ManuallySubmit( column_indices[ i ] );
	}
	void Table_Header_ManuallySubmit_AppendHelpMarker( const int column_index, const char* help_string );

	/* Helper to display a little (?) mark which shows a tooltip when hovered. */
	void HelpMarker( const char* desc, const int wrap = 35.0f );

	void CenterText( const char* text );
	void CenterItem( const int item_width );
	void CenterCheckbox();

	void ImmutableCheckbox( const char* text, const bool is_enabled );

	bool BeginOverlay( const char* window_name, const char* name, 
					   const HorizontalPosition horizontal_positioning, const VerticalPosition vertical_positioning,
					   bool* p_open = ( bool* )0, 
					   const float alpha = 0.35f );
	bool BeginOverlay( const char* window_name, const char* name,
					   const ImVec2 pos,
					   bool* p_open = ( bool* )0,
					   const float alpha = 0.35f );
	void EndOverlay();

	/* Only works with same width items. */
	void SetNextItemRightAligned( const int item_no_starting_from_right, const float item_width );
	
	void SetNextWindowPos( const HorizontalPosition horizontal_positioning, const VerticalPosition vertical_positioning,
						   const ImGuiCond condition = ImGuiCond_Always );

	/* https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353 */
	void BeginGroupPanel( const char* name = nullptr, bool* is_enabled = nullptr, const ImVec2& size = ImVec2( 0.0f, 0.0f ) );
	/* https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353 */
	void EndGroupPanel( bool* is_enabled = nullptr );

	void BeginDisabledButInteractable();
	void EndDisabledButInteractable();

	void DrawRoundedRectText( const char* text, const ImU32 color, const float thickness = 2.0f, const float rounding = 3.0f );
	void DrawRainbowRectText( const char* text, const ImU32 text_color = ImColor{ 0.0f, 0.0f, 0.0f, 1.0f } );

	void DrawArrow( ImDrawList* draw_list, float size = -1.0f, const ImU32 color = ImColor{ 0.75f, 0.75f, 0.75f, 1.0f }, const bool advance_cursor = true );

	void DrawShadedSphere( ImDrawList* draw_list, const ImU32 shade_color, const ImU32 specular_color, const float radius, const bool advance_cursor = true );
	bool DrawShadedSphereComboButton( const char* name,
									  int* current_index,
									  const std::initializer_list< const char* > option_strings,
									  const ImU32 shade_color    = ImColor{ 0.2f, 0.2f, 0.2f, 1.0f },
									  const ImU32 specular_color = ImColor{ 0.75f, 0.75f, 0.75f, 1.0f } );
}
