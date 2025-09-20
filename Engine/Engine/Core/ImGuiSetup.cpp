// Engine Includes.
#include "ImGuiSetup.h"
#include "Platform.h"
#include "../Asset/Paths.h"

// Vendor Includes.
#include <ImGui/imgui.h>
#include <ImGui/backends/imgui_impl_glfw.h>
#include <ImGui/backends/imgui_impl_opengl3.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Engine::ImGuiSetup
{
	void Initialize( const bool enable_gamma_correction )
	{
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // Setup Platform/Renderer backends.
        ImGui_ImplGlfw_InitForOpenGL( reinterpret_cast< GLFWwindow* >( Platform::GetWindowHandle() ), true );
        ImGui_ImplOpenGL3_Init();

        AddFonts();

        SetStyle( enable_gamma_correction );
	}

    void Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
    }

    void AddFonts()
    {
        ImGuiIO& io = ImGui::GetIO();

        float base_font_size = 18.0f;
        float icon_font_size = base_font_size /** 2.0f / 3.0f*/;

        // The ranges array is not copied by the AddFont* functions and is used lazily so it NEEDS TO BE available at the time of building or calling GetTexDataAsRGBA32().

        static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 }; // Will not be copied by AddFont* so keep in scope.
        ImFontConfig icons_config;
        icons_config.MergeMode            = true;
        icons_config.PixelSnapH           = true;
        icons_config.GlyphMinAdvanceX     = icon_font_size;
        icons_config.FontDataOwnedByAtlas = false;

        ImFontGlyphRangesBuilder font_glyph_ranges_builder;
        //font_glyph_ranges_builder.AddChar( 0x00A6 ); // Left here as example, for future reference.
        font_glyph_ranges_builder.AddRanges( io.Fonts->GetGlyphRangesDefault() );
        static ImVector< ImWchar > glyph_ranges;
        font_glyph_ranges_builder.BuildRanges( &glyph_ranges );

        io.Fonts->Clear();
        io.Fonts->AddFontFromFileTTF( ENGINE_FONT_PATH_ABSOLUTE( "JetBrainsMono-Regular.ttf" ), 18, nullptr, glyph_ranges.Data );
        /* Merge in icons from icon font(s): */
        io.Fonts->AddFontFromFileTTF( ENGINE_FONT_PATH_ABSOLUTE( "Font-Awesome/" FONT_ICON_FILE_NAME_FAS ), icon_font_size, &icons_config, icons_ranges ); // Merge into JetBrainsMono-Regular (18).

        io.Fonts->Build();
    }

    void SetStyle( const bool enable_gamma_correction )
    {
        ImGuiStyle& style = ImGui::GetStyle();

        using namespace Engine::Math;

        /*
         * As ImGui is not gamma-aware, we'll just gamma-correct the ImGui style colors here and be done with it.
         */

        auto GammaCorrectedColor = [ & ]( const float source_red, const float source_green, const float source_blue, const float source_alpha )
        {
            if( not enable_gamma_correction )
                return ImVec4( source_red, source_green, source_blue, source_alpha );

            auto GammaCorrectedColorChannel = []( const float source )
            {
                // Use the formula from the GL docs instead of naively raising the source to 2.2f: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexImage2D.xhtml
                return source <= 0.04045f
                        ? source / 12.92f
                        : Pow( ( source + 0.055f ) / 1.055f, 2.4f );
            };

            return ImVec4( GammaCorrectedColorChannel( source_red ), GammaCorrectedColorChannel( source_green ), GammaCorrectedColorChannel( source_blue ), source_alpha );
        };

        ImVec4* colors                               = style.Colors;
        colors[ ImGuiCol_Text ]                      = GammaCorrectedColor( 0.92f, 0.92f, 0.92f, 1.00f );
        colors[ ImGuiCol_TextDisabled ]              = GammaCorrectedColor( 0.53f, 0.53f, 0.53f, 1.00f );
        colors[ ImGuiCol_WindowBg ]                  = GammaCorrectedColor( 0.19f, 0.19f, 0.19f, 1.00f );
        colors[ ImGuiCol_ChildBg ]                   = GammaCorrectedColor( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_PopupBg ]                   = GammaCorrectedColor( 0.08f, 0.08f, 0.08f, 0.94f );
        colors[ ImGuiCol_Border ]                    = GammaCorrectedColor( 0.37f, 0.37f, 0.41f, 0.50f );
        colors[ ImGuiCol_BorderShadow ]              = GammaCorrectedColor( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ ImGuiCol_FrameBg ]                   = GammaCorrectedColor( 0.11f, 0.11f, 0.11f, 0.78f );
        colors[ ImGuiCol_FrameBgHovered ]            = GammaCorrectedColor( 0.32f, 0.24f, 0.00f, 0.75f );
        colors[ ImGuiCol_FrameBgActive ]             = GammaCorrectedColor( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_TitleBg ]                   = GammaCorrectedColor( 0.14f, 0.14f, 0.14f, 1.00f );
        colors[ ImGuiCol_TitleBgActive ]             = GammaCorrectedColor( 0.31f, 0.31f, 0.31f, 1.00f );
        colors[ ImGuiCol_TitleBgCollapsed ]          = GammaCorrectedColor( 0.14f, 0.14f, 0.14f, 1.00f );
        colors[ ImGuiCol_MenuBarBg ]                 = GammaCorrectedColor( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_ScrollbarBg ]               = GammaCorrectedColor( 0.16f, 0.16f, 0.16f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrab ]             = GammaCorrectedColor( 0.25f, 0.25f, 0.25f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrabHovered ]      = GammaCorrectedColor( 0.41f, 0.41f, 0.41f, 1.00f );
        colors[ ImGuiCol_ScrollbarGrabActive ]       = GammaCorrectedColor( 0.51f, 0.51f, 0.51f, 1.00f );
        colors[ ImGuiCol_CheckMark ]                 = GammaCorrectedColor( 0.96f, 0.57f, 0.06f, 0.71f );
        colors[ ImGuiCol_SliderGrab ]                = GammaCorrectedColor( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_SliderGrabActive ]          = GammaCorrectedColor( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_Button ]                    = GammaCorrectedColor( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_ButtonHovered ]             = GammaCorrectedColor( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_ButtonActive ]              = GammaCorrectedColor( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_Header ]                    = GammaCorrectedColor( 0.83f, 0.60f, 0.00f, 0.75f );
        colors[ ImGuiCol_HeaderHovered ]             = GammaCorrectedColor( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_HeaderActive ]              = GammaCorrectedColor( 1.00f, 0.78f, 0.10f, 0.75f );
        colors[ ImGuiCol_Separator ]                 = GammaCorrectedColor( 0.37f, 0.37f, 0.41f, 0.50f );
        colors[ ImGuiCol_SeparatorHovered ]          = GammaCorrectedColor( 0.10f, 0.40f, 0.75f, 0.78f );
        colors[ ImGuiCol_SeparatorActive ]           = GammaCorrectedColor( 0.10f, 0.40f, 0.75f, 1.00f );
        colors[ ImGuiCol_ResizeGrip ]                = GammaCorrectedColor( 0.65f, 0.47f, 0.00f, 0.75f );
        colors[ ImGuiCol_ResizeGripHovered ]         = GammaCorrectedColor( 0.92f, 0.69f, 0.00f, 0.75f );
        colors[ ImGuiCol_ResizeGripActive ]          = GammaCorrectedColor( 1.00f, 0.78f, 0.09f, 0.75f );
        colors[ ImGuiCol_TabHovered ]                = GammaCorrectedColor( 1.00f, 1.00f, 1.00f, 0.50f );
        colors[ ImGuiCol_Tab ]                       = GammaCorrectedColor( 0.10f, 0.10f, 0.10f, 1.00f );
        colors[ ImGuiCol_TabSelected ]               = GammaCorrectedColor( 0.43f, 0.43f, 0.43f, 0.75f );
        colors[ ImGuiCol_TabSelectedOverline ]       = GammaCorrectedColor( 1.00f, 0.73f, 0.00f, 0.00f );
        colors[ ImGuiCol_TabDimmed ]                 = GammaCorrectedColor( 0.06f, 0.06f, 0.06f, 0.75f );
        colors[ ImGuiCol_TabDimmedSelected ]         = GammaCorrectedColor( 0.33f, 0.33f, 0.33f, 0.75f );
        colors[ ImGuiCol_TabDimmedSelectedOverline ] = GammaCorrectedColor( 1.00f, 0.00f, 0.00f, 1.00f );
        colors[ ImGuiCol_PlotLines ]                 = GammaCorrectedColor( 0.90f, 0.70f, 0.00f, 1.00f );
        colors[ ImGuiCol_PlotLinesHovered ]          = GammaCorrectedColor( 1.00f, 0.60f, 0.00f, 1.00f );
        colors[ ImGuiCol_PlotHistogram ]             = GammaCorrectedColor( 0.90f, 0.70f, 0.00f, 1.00f );
        colors[ ImGuiCol_PlotHistogramHovered ]      = GammaCorrectedColor( 1.00f, 0.60f, 0.00f, 1.00f );
        colors[ ImGuiCol_TableHeaderBg ]             = GammaCorrectedColor( 0.35f, 0.35f, 0.35f, 1.00f );
        colors[ ImGuiCol_TableBorderStrong ]         = GammaCorrectedColor( 0.27f, 0.27f, 0.27f, 1.00f );
        colors[ ImGuiCol_TableBorderLight ]          = GammaCorrectedColor( 0.27f, 0.27f, 0.27f, 1.00f );
        colors[ ImGuiCol_TableRowBg ]                = GammaCorrectedColor( 0.00f, 0.00f, 0.00f, 0.00f );
        colors[ ImGuiCol_TableRowBgAlt ]             = GammaCorrectedColor( 1.00f, 1.00f, 1.00f, 0.06f );
        colors[ ImGuiCol_TextLink ]                  = GammaCorrectedColor( 0.26f, 0.59f, 0.98f, 1.00f );
        colors[ ImGuiCol_TextSelectedBg ]            = GammaCorrectedColor( 0.26f, 0.59f, 0.98f, 0.35f );
        colors[ ImGuiCol_DragDropTarget ]            = GammaCorrectedColor( 1.00f, 1.00f, 0.00f, 0.90f );
        colors[ ImGuiCol_NavHighlight ]              = GammaCorrectedColor( 0.26f, 0.59f, 0.98f, 1.00f );
        colors[ ImGuiCol_NavWindowingHighlight ]     = GammaCorrectedColor( 1.00f, 1.00f, 1.00f, 0.70f );
        colors[ ImGuiCol_NavWindowingDimBg ]         = GammaCorrectedColor( 0.80f, 0.80f, 0.80f, 0.20f );
        colors[ ImGuiCol_ModalWindowDimBg ]          = GammaCorrectedColor( 0.80f, 0.80f, 0.80f, 0.35f );

        /* Main: */
        style.WindowPadding     = ImVec2( 8, 8 );
        style.FramePadding      = ImVec2( 4, 3 );
        style.ItemSpacing       = ImVec2( 8, 4 );
        style.ItemInnerSpacing  = ImVec2( 4, 4 );
        style.IndentSpacing     = 20.0f;
        style.ScrollbarSize     = 16.0f;
        style.GrabMinSize       = 12.0f;

        /* Borders: */
        style.WindowBorderSize   = 0.0f;
        style.ChildBorderSize    = 0.0f;
        style.PopupBorderSize    = 0.0f;
        style.FrameBorderSize    = 0.0f;
        style.TabBorderSize      = 0.0f;
        style.TabBarBorderSize   = 2.0f;
        style.TabBarOverlineSize = 0.0f;

        /* Rounding: */
        style.WindowRounding    = 4.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 4.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 4.0f;

        /* Tables: */
        style.CellPadding                 = ImVec2( 4, 2 );
        style.TableAngledHeadersAngle     = 35.0f;
        style.TableAngledHeadersTextAlign = ImVec2( 0.5f, 0.0f );

        /* Widgets: */
        style.WindowTitleAlign        = ImVec2( 0.0f, 0.5f );
        style.ButtonTextAlign         = ImVec2( 0.5f, 0.5f );
        style.SeparatorTextBorderSize = 2.0f;
        style.SeparatorTextAlign      = ImVec2( 0.0f, 0.5f );
        style.SeparatorTextPadding    = ImVec2( 4, 2 );

        /* Disable collapse button: */
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }
}