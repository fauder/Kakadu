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

    bool BeginOverlay( const char* window_name, const char* name,
                       const HorizontalPosition horizontal_positioning, const VerticalPosition vertical_positioning,
                       bool* p_open, const float alpha )
    {
		constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		
        const auto window = ImGui::FindWindowByName( window_name );

        ASSERT_EDITOR_ONLY( window != nullptr );
        ASSERT_EDITOR_ONLY( not ( horizontal_positioning == HorizontalPosition::CENTER && vertical_positioning == VerticalPosition::CENTER ) &&
							"Overlay at the center of the viewport is not allowed!" );

        constexpr float pad = 10.0f;
		ImVec2 window_pos, window_pos_pivot;

		window_pos.x = window->Pos.x + 
			            ( horizontal_positioning == HorizontalPosition::LEFT
			              ? pad
			              : horizontal_positioning == HorizontalPosition::CENTER
			                ? window->Size.x / 2.0f
			                : window->Size.x - pad );
		window_pos.y = window->Pos.y +
                        ( vertical_positioning == VerticalPosition::TOP
                            ? pad + window->TitleBarHeight
                            : vertical_positioning == VerticalPosition::CENTER
                                ? window->Size.y / 2.0f
                                : window->Size.y - pad );
		window_pos_pivot.x = ( int )horizontal_positioning * 0.5f;
		window_pos_pivot.y = ( int )vertical_positioning * 0.5f;
		ImGui::SetNextWindowPos( window_pos, ImGuiCond_Always, window_pos_pivot );

		ImGui::SetNextWindowBgAlpha( alpha );
        if( ImGui::Begin( name, p_open, window_flags ) )
        {
            if( name[ 0 ] != '#' )
                ImGui::TextUnformatted( name );

            return true;
        }

        return false;
    }

    bool BeginOverlay( const char* window_name, const char* name, const ImVec2 pos, bool* p_open, const float alpha )
    {
        constexpr ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize | 
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
		
        const auto window = ImGui::FindWindowByName( window_name );

        ASSERT_EDITOR_ONLY( window != nullptr );

        ImGui::SetNextWindowPos( pos, ImGuiCond_Always );

		ImGui::SetNextWindowBgAlpha( alpha );
        if( ImGui::Begin( name, p_open, window_flags ) )
        {
            if( name[ 0 ] != '#' )
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

	void SetNextWindowPos( const HorizontalPosition horizontal_positioning, const VerticalPosition vertical_positioning, const ImGuiCond condition )
	{
		const auto& io = ImGui::GetIO();
		const auto horizontal_position = horizontal_positioning == HorizontalPosition::RIGHT
			? io.DisplaySize.x
			: horizontal_positioning == HorizontalPosition::CENTER
				? io.DisplaySize.x / 2.0f
				: 0.0f;
		const auto vertical_position = vertical_positioning == VerticalPosition::BOTTOM
			? io.DisplaySize.y
			: vertical_positioning == VerticalPosition::CENTER
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
    void DrawRoundedRectText( const char* text, const ImU32 color, const float thickness, const float rounding )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const auto& style = ImGui::GetStyle();

        const ImVec2 pos       = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize( text );
        const ImVec2 size      = { 2.2f * style.ItemInnerSpacing.x + text_size.x, 1.1f * text_size.y };

        draw_list->AddRect( pos, pos + size, color, rounding, ImDrawFlags_None, thickness );
        ImGui::PushStyleColor( ImGuiCol_Text, color );
        ImGui::SetCursorScreenPos( pos + ImVec2{ 1.1f * style.ItemInnerSpacing.x, 0.0f } );
        ImGui::TextUnformatted( text );
        ImGui::PopStyleColor();
    }

	void DrawRainbowRectText( const char* text, const ImU32 text_color )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const auto& style = ImGui::GetStyle();

        const ImVec2 pos       = ImGui::GetCursorScreenPos();
        const ImVec2 text_size = ImGui::CalcTextSize( text );
        const ImVec2 size      = { 2.2f * style.ItemInnerSpacing.x + text_size.x, 1.1f * text_size.y };

        draw_list->AddRectFilledMultiColor( pos, pos + size, IM_COL32( 0, 0, 0, 255 ), IM_COL32( 255, 0, 0, 255 ), IM_COL32( 255, 255, 0, 255 ), IM_COL32( 0, 255, 0, 255 ) );
        ImGui::PushStyleColor( ImGuiCol_Text, text_color );
        ImGui::SetCursorScreenPos( pos + ImVec2{ 1.1f * style.ItemInnerSpacing.x, 0.0f } );
        ImGui::TextUnformatted( text );
        ImGui::PopStyleColor();
    }

    void DrawArrow( ImDrawList* draw_list, float size, const ImU32 color, const bool advance_cursor )
    {
        if( size < 0.0f )
            size = ImGui::CalcTextSize( "A" ).x * 1.25f;

        const ImVec2 pos = ImGui::GetCursorScreenPos() + ImVec2{ size * 0.5f, 0.0f };

        ImVec2 p1 = { pos.x - size * 0.5f, pos.y + size * 0.5f };
        ImVec2 p2 = { pos.x + size * 0.5f, pos.y + size * 0.5f };
        ImVec2 p3 = { pos.x, pos.y + size * ( 0.5f + 0.6f ) };

        draw_list->AddTriangleFilled( p1, p2, p3, color );

        if( advance_cursor )
            ImGui::SetCursorScreenPos( pos + ImVec2{ size, 0.0f } );
    }

    void DrawShadedSphere( ImDrawList* draw_list, const ImU32 shade_color, const ImU32 specular_color, const float radius, const bool advance_cursor )
    {
        const auto& style = ImGui::GetStyle();

        const ImVec2 pos                = ImGui::GetCursorScreenPos();
        const ImVec2 size               = ImVec2{ 2.0f * ( radius + style.ItemInnerSpacing.x ), style.ItemInnerSpacing.y + radius };
        
        const ImVec2 center = pos + ImVec2{ radius, radius };

        draw_list->AddCircleFilled( center, radius, shade_color );

        const ImVec2 highlight_center = { center.x - radius * 0.3f, center.y - radius * 0.4f };
        const ImVec2 highlight_size   = { radius * 0.7f, radius * 0.5f };

        constexpr float highlight_rot = -0.7f; // -45 degrees.

        draw_list->AddEllipseFilled( highlight_center, highlight_size, specular_color, highlight_rot );

        if( advance_cursor )
            ImGui::SetCursorScreenPos( pos + ImVec2{ size.x, 0.0f } );
    }

    bool DrawShadedSphereComboButton( const char* name,
                                      int* current_index,
                                      const std::initializer_list< const char* > option_strings,
                                      const ImU32 shade_color, const ImU32 specular_color )
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        const float radius = ImGui::CalcTextSize( "A" ).x * 1.15f;

        const auto pos_start = ImGui::GetCursorScreenPos();

        DrawShadedSphere( draw_list, shade_color, specular_color, radius );
        DrawArrow( draw_list );

        const auto width = ImGui::GetCursorScreenPos().x - pos_start.x;

        ImGui::SetCursorScreenPos( pos_start );

        ImGui::InvisibleButton( name, { width, radius * 2 } );

        bool result = false;

        if( ImGui::BeginPopupContextItem( name ) )
        {
            int index = 0;
            for( const auto option_string : option_strings )
            {
                if( option_string[ 0 ] == '_' )
                    ImGui::Separator();
                else 
                {
                    if( ImGui::Selectable( option_string ) )
                    {
                        *current_index = index;
                        result = true;
                    }

                    index++;
                }
            }

            ImGui::EndPopup();
        }

        if( ImGui::IsItemClicked( ImGuiMouseButton_Left ) )
            ImGui::OpenPopup( name );

        return result;
    }
}