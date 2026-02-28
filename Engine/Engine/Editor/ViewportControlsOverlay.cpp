// Engine Includes.
#include "ViewportControlsOverlay.h"
#include "Core/ImGuiUtility.h"
#include "Core/ImGuiSetup.h"
#include "Graphics/Renderer.h"

namespace Engine::Editor
{
	internal_function void RenderShadingModeCombobox( Context& editor_context, const EditorShadingMode editor_shading_mode )
	{
		int option = ( int )editor_shading_mode;
		if( ImGuiUtility::DrawShadedSphereComboButton( "ShadingMode", &option,
													   {
														   "Shaded",
														   "Wireframe",
														   "Shaded Wireframe",
														   "___",
														   "Texture Coordinates",
														   "Geometry Tangents",
														   "Geometry Bitangents",
														   "Geometry Normals",
														   "Debug Vectors",
														   "Shading Normals"
													   } ) )
		{
			Command command{ .type = Command::Type::Renderer_ChangeEditorShadingMode };
			std::memcpy( command.payload.data(), &option, sizeof( option ) );
			editor_context.commands_queue.push( command );
		}
	}

	void RenderViewportControlsOverlay( Context& editor_context, const Renderer& renderer )
	{
		if( ImGuiUtility::BeginOverlay( editor_context.viewport_panel.imgui_window_name.c_str(), "##ViewportControls",
										ImGuiUtility::HorizontalPosition::LEFT, ImGuiUtility::VerticalPosition::TOP,
										&editor_context.show_frame_statistics_overlay ) )
		{
			RenderShadingModeCombobox( editor_context, renderer.GetEditorShadingMode() );
		}

		ImGuiUtility::EndOverlay();
	}
}
