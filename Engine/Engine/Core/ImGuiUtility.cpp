// Engine Includes.
#include "Assertion.h"
#include "ImGuiUtility.h"

// Vendor Includes.
#include <ImGui/imgui_internal.h>

namespace Engine::ImGuiUtility
{
	void Table_Header_ManuallySubmit( const int column_index )
	{
		ImGui::TableSetColumnIndex( column_index );
		const char* column_name = ImGui::TableGetColumnName( column_index ); // Retrieve name passed to TableSetupColumn().
		ImGui::PushID( column_index );
		//ImGui::Indent( ( float )font_width );
		ImGui::TableHeader( column_name );
		//ImGui::Unindent( ( float )font_width );
		ImGui::PopID();
	}

	void Table_Header_ManuallySubmit_AppendHelpMarker( const int column_index, const char* help_string )
	{
		/* Due to a Table API limitation, the help marker can NOT be placed AFTER the header name. Therefore it has to be placed before. */

		ImGui::TableSetColumnIndex( column_index );
		const char* column_name = ImGui::TableGetColumnName( column_index ); // Retrieve name passed to TableSetupColumn().
		ImGui::PushID( column_index );
		ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0, 0 ) );
		ImGuiUtility::HelpMarker( help_string );
		ImGui::PopStyleVar();
		ImGui::SameLine( 0.0f, ImGui::GetStyle().ItemInnerSpacing.x );
		ImGui::TableHeader( column_name );
		ImGui::PopID();
	}

	void HelpMarker( const char* desc, const int wrap )
	{
		ImGui::TextDisabled( "(?)" );
		if( ImGui::IsItemHovered() )
		{
			ImGui::BeginTooltip();
			if( wrap )
				ImGui::PushTextWrapPos( ImGui::GetFontSize() * wrap );
			ImGui::TextUnformatted( desc );
			if( wrap )
				ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

    void CenterText( const char* text )
    {
        const float available_width = ImGui::GetContentRegionAvail().x;
        const float text_width      = ImGui::CalcTextSize( text ).x;
        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( available_width - text_width ) * 0.5f );
    }

    void CenterItem( const int item_width )
    {
        const float available_width = ImGui::GetContentRegionAvail().x;
        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( available_width - item_width ) * 0.5f );
    }

    void CenterCheckbox()
    {
        const float available_width = ImGui::GetContentRegionAvail().x;
        const float checkbox_width  = ImGui::GetFrameHeight(); // Height == width for the checkbox.
        ImGui::SetCursorPosX( ImGui::GetCursorPosX() + ( available_width - checkbox_width ) * 0.5f );
    }

    void ImmutableCheckbox( const char* text, const bool is_enabled )
    {
        ImGui::BeginDisabled();
        bool enabled = is_enabled;
        ImGui::Checkbox( text, &enabled );
        ImGui::EndDisabled();
    }

    bool BeginOverlay( const char* window_name, const char* name, bool* p_open, const WindowCorner window_corner, const float alpha )
    {
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		
        const auto window = ImGui::FindWindowByName( window_name );

        ASSERT_EDITOR_ONLY( window != nullptr );

        constexpr float pad = 10.0f;
		ImVec2 window_pos, window_pos_pivot;

        const bool is_right  = window_corner == WindowCorner::TOP_RIGHT || window_corner == WindowCorner::BOTTOM_RIGHT;
        const bool is_bottom = window_corner == WindowCorner::BOTTOM_LEFT || window_corner == WindowCorner::BOTTOM_RIGHT;
		window_pos.x         = is_right  ? ( window->Pos.x + window->Size.x - pad ) : ( window->Pos.x + pad );
		window_pos.y         = is_bottom ? ( window->Pos.y + window->Size.y - pad ) : ( window->Pos.y + pad + window->TitleBarHeight );
		window_pos_pivot.x   = is_right  ? 1.0f : 0.0f;
		window_pos_pivot.y   = is_bottom ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos( window_pos, ImGuiCond_Always, window_pos_pivot );
		window_flags |= ImGuiWindowFlags_NoMove;

		ImGui::SetNextWindowBgAlpha( alpha );
        if( ImGui::Begin( name, p_open, window_flags ) )
        {
            ImGui::TextUnformatted( name );
            return true;
        }

        return false;
    }

    void EndOverlay()
    {
        ImGui::End();
    }

    /* Only works with same width items. */
    void SetNextItemRightAligned( const int item_no_starting_from_right, const float item_width )
    {
        ImGui::SetCursorPosX( ImGui::GetWindowWidth() - item_no_starting_from_right * ( ImGui::GetStyle().ItemSpacing.x + item_width ) );
    }

	void SetNextWindowPos( const HorizontalWindowPositioning horizontal_positioning, const VerticalWindowPositioning vertical_positioning, const ImGuiCond condition )
	{
		const auto& io = ImGui::GetIO();
		const auto horizontal_position = horizontal_positioning == HorizontalWindowPositioning::RIGHT
			? io.DisplaySize.x
			: horizontal_positioning == HorizontalWindowPositioning::CENTER
				? io.DisplaySize.x / 2.0f
				: 0.0f;
		const auto vertical_position = vertical_positioning == VerticalWindowPositioning::BOTTOM
			? io.DisplaySize.y
			: vertical_positioning == VerticalWindowPositioning::CENTER
				? io.DisplaySize.y / 2.0f
				: 0.0f;
		const auto horizontal_pivot = ( float )horizontal_positioning / 2.0f; // Map to [+1, 0] range.
		const auto vertical_pivot   = ( float )vertical_positioning   / 2.0f; // Map to [+1, 0] range.

		ImGui::SetNextWindowPos( { horizontal_position, vertical_position }, condition, { horizontal_pivot, vertical_pivot } );
	}

    static ImVector<ImRect> s_GroupPanelLabelStack;

    /* https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353 */
    void BeginGroupPanel( const char* name, bool* is_enabled, const ImVec2& size )
    {
        ImGui::BeginGroup();

        const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

        const auto frameHeight = ImGui::GetFrameHeight();
        ImGui::BeginGroup();

        ImVec2 effectiveSize = size;
        if( size.x < 0.0f )
            effectiveSize.x = ImGui::GetContentRegionAvail().x;
        else
            effectiveSize.x = size.x;
        ImGui::Dummy( ImVec2( effectiveSize.x, 0.0f ) );

        ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
        ImGui::SameLine( 0.0f, 0.0f );
        ImGui::BeginGroup();
        ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
        ImGui::SameLine( 0.0f, 0.0f );
        if( is_enabled != nullptr )
        {
            ImGui::Checkbox( name, is_enabled );
            if( not *is_enabled )
				ImGui::BeginDisabled();
        }
        else if( name != nullptr )
            ImGui::TextUnformatted( name );
        const auto labelMin = ImGui::GetItemRectMin();
        const auto labelMax = ImGui::GetItemRectMax();
        ImGui::SameLine( 0.0f, 0.0f );
        ImGui::Dummy( ImVec2( 0.0, frameHeight + itemSpacing.y ) );
        ImGui::BeginGroup();

        //ImGui::GetWindowDrawList()->AddRect(labelMin, labelMax, IM_COL32(255, 0, 255, 255));

        ImGui::PopStyleVar( 2 );

    #if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x -= frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x -= frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x -= frameHeight * 0.5f;
    #else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x -= frameHeight * 0.5f;
    #endif
        ImGui::GetCurrentWindow()->Size.x -= frameHeight;

        const auto itemWidth = ImGui::CalcItemWidth();
        ImGui::PushItemWidth( ImMax( 0.0f, itemWidth - frameHeight ) );

        s_GroupPanelLabelStack.push_back( ImRect( labelMin, labelMax ) );
    }

    /* https://github.com/ocornut/imgui/issues/1496#issuecomment-655048353 */
    void EndGroupPanel( bool* is_enabled )
    {
        if( is_enabled != nullptr && not *is_enabled )
            ImGui::EndDisabled();

        ImGui::PopItemWidth();

        const auto itemSpacing = ImGui::GetStyle().ItemSpacing;

        ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 0.0f, 0.0f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_ItemSpacing, ImVec2( 0.0f, 0.0f ) );

        const auto frameHeight = ImGui::GetFrameHeight();

        ImGui::EndGroup();

        //ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(0, 255, 0, 64), 4.0f);

        ImGui::EndGroup();

        ImGui::SameLine( 0.0f, 0.0f );
        ImGui::Dummy( ImVec2( frameHeight * 0.5f, 0.0f ) );
        ImGui::Dummy( ImVec2( 0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y ) );

        ImGui::EndGroup();

        auto itemMin = ImGui::GetItemRectMin();
        auto itemMax = ImGui::GetItemRectMax();
        //ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(255, 0, 0, 64), 4.0f);

        auto labelRect = s_GroupPanelLabelStack.back();
        s_GroupPanelLabelStack.pop_back();

        ImVec2 halfFrame = ImVec2( frameHeight * 0.25f, frameHeight ) * 0.5f;
        ImRect frameRect = ImRect( itemMin + halfFrame, itemMax - ImVec2( halfFrame.x, 0.0f ) );
        labelRect.Min.x -= itemSpacing.x;
        labelRect.Max.x += itemSpacing.x;
        for( int i = 0; i < 4; ++i )
        {
            switch( i )
            {
                // left half-plane
                case 0: ImGui::PushClipRect( ImVec2( -FLT_MAX, -FLT_MAX ), ImVec2( labelRect.Min.x, FLT_MAX ), true ); break;
                // right half-plane
                case 1: ImGui::PushClipRect( ImVec2( labelRect.Max.x, -FLT_MAX ), ImVec2( FLT_MAX, FLT_MAX ), true ); break;
                // top
                case 2: ImGui::PushClipRect( ImVec2( labelRect.Min.x, -FLT_MAX ), ImVec2( labelRect.Max.x, labelRect.Min.y ), true ); break;
                // bottom
                case 3: ImGui::PushClipRect( ImVec2( labelRect.Min.x, labelRect.Max.y ), ImVec2( labelRect.Max.x, FLT_MAX ), true ); break;
            }

            ImGui::GetWindowDrawList()->AddRect(
                frameRect.Min, frameRect.Max,
                ImColor( ImGui::GetStyleColorVec4( ImGuiCol_Border ) ),
                halfFrame.x );

            ImGui::PopClipRect();
        }

        ImGui::PopStyleVar( 2 );

    #if IMGUI_VERSION_NUM >= 17301
        ImGui::GetCurrentWindow()->ContentRegionRect.Max.x += frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->WorkRect.Max.x += frameHeight * 0.5f;
        ImGui::GetCurrentWindow()->InnerRect.Max.x += frameHeight * 0.5f;
    #else
        ImGui::GetCurrentWindow()->ContentsRegionRect.Max.x += frameHeight * 0.5f;
    #endif
        ImGui::GetCurrentWindow()->Size.x += frameHeight;

        ImGui::Dummy( ImVec2( 0.0f, 0.0f ) );

        ImGui::EndGroup();
    }

    void BeginDisabledButInteractable()
    {
         ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );
    }

    void EndDisabledButInteractable()
    {
        ImGui::PopStyleColor();
    }
    void DrawRoundedRectText( const char* text, const ImVec4& color, const float thickness, const float rounding )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const auto& style = ImGui::GetStyle();

        const ImVec2 pos       = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize( text );
        const ImVec2 size      = { 2.2f * style.ItemInnerSpacing.x + text_size.x, 1.1f * text_size.y };
        const ImU32  color_u32 = ImColor( color );

        draw_list->AddRect( pos, pos + size, color_u32, rounding, ImDrawFlags_None, thickness );
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::SetCursorScreenPos( pos + ImVec2{ 1.1f * style.ItemInnerSpacing.x, 0.0f } );
        ImGui::TextUnformatted( text );
        ImGui::PopStyleColor();
    }

	void DrawRainbowRectText( const char* text, const ImVec4& text_color )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const auto& style = ImGui::GetStyle();

        const ImVec2 pos       = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize( text );
        const ImVec2 size      = { 2.2f * style.ItemInnerSpacing.x + text_size.x, 1.1f * text_size.y };
        const ImU32  color_u32 = ImColor( text_color );

        draw_list->AddRectFilledMultiColor( pos, pos + size, IM_COL32( 0, 0, 0, 255 ), IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ), IM_COL32( 0, 255, 0, 255 ) );
        ImGui::PushStyleColor( ImGuiCol_Text, color_u32 );
        ImGui::SetCursorScreenPos( pos + ImVec2{ 1.1f * style.ItemInnerSpacing.x, 0.0f } );
        ImGui::TextUnformatted( text );
        ImGui::PopStyleColor();
    }
}