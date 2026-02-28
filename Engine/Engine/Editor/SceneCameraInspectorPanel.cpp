// Engine Includes.
#include "SceneCameraInspectorPanel.h"
#include "Core/ImGuiDrawer.hpp"
#include "Core/ImGuiUtility.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Kakadu::Editor
{
	void RenderSceneCameraInspectorPanel( SceneCamera& scene_camera, const Vector2I viewport_resolution )
	{
		if( ImGui::Begin( ICON_FA_VIDEO " Scene Camera" ) )
		{
			if( ImGui::BeginTabBar( "TabBar" ) )
			{
				if( ImGui::BeginTabItem( ICON_FA_UP_DOWN_LEFT_RIGHT " Transform" ) )
				{
					const auto& style = ImGui::GetStyle();

					/* Row 1: */
					const float button_width( ImGui::CalcTextSize( ICON_FA_ARROWS_ROTATE " XXXXXX" ).x + style.ItemInnerSpacing.x );
					ImGui::SetCursorPosX( button_width );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Top" ) )
						scene_camera.SwitchView( SceneCamera::View::TOP );

					/* Row 2: */
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Left" ) )
						scene_camera.SwitchView( SceneCamera::View::LEFT );
					ImGui::SameLine();
					ImGui::SetCursorPosX( button_width );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Front" ) )
						scene_camera.SwitchView( SceneCamera::View::FRONT );
					ImGui::SameLine();
					ImGui::SetCursorPosX( button_width * 2 );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Right" ) )
						scene_camera.SwitchView( SceneCamera::View::RIGHT );

					/* Row 3: */
					ImGui::SetCursorPosX( button_width );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Back" ) )
						scene_camera.SwitchView( SceneCamera::View::BACK );

					/* Row 4: */
					ImGui::SetCursorPosX( button_width );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Bottom" ) )
						scene_camera.SwitchView( SceneCamera::View::BOTTOM );

					ImGui::NewLine();

					/* Row 5: */
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Custom (1)" ) )
						scene_camera.SwitchView( SceneCamera::View::CUSTOM_1 );
					if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Camera" ) )
						scene_camera.Reset();

					ImGui::Checkbox( "Animate (Rotate) Camera", &scene_camera.animation_is_enabled );
					if( scene_camera.animation_is_enabled )
						ImGui::SliderFloat( "Camera Orbit Radius", &scene_camera.animation_orbit_radius, 0.0f, 50.0f );

					Kakadu::ImGuiDrawer::Draw( scene_camera.transform, Kakadu::Transform::Mask::NoScale );

					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Projection" ) )
				{
					Kakadu::ImGuiUtility::BeginGroupPanel();
					{
						if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Projection" ) )
							scene_camera.ResetProjection();

						if( Kakadu::ImGuiDrawer::Draw( scene_camera.camera, "Main Camera", true ) )
							scene_camera.RecalculateProjectionParameters( viewport_resolution );
					}
					Kakadu::ImGuiUtility::EndGroupPanel();

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}

		ImGui::End();
	}
}
