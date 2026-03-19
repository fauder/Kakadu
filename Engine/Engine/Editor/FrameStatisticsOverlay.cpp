#define IMGUI_DEFINE_MATH_OPERATORS

// Engine Includes.
#include "FrameStatisticsOverlay.h"
#include "Core/ImGuiUtility.h"
#include "Graphics/Color.hpp"
#include "Math/VectorConversion.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Kakadu::Editor
{
	void FrameStatisticsOverlay::Update( const float time_delta_real )
	{
		/*
		 * Calculate frame statistics:
		 */

		const float fps        = 1.0f / time_delta_real;
		const float frame_time = time_delta_real * 1000.0f;

		/* Calculate rolling avg. fps & frame time: */
		{
			/* Since only 1 value in the ring buffer changes every frame, no need to re-calculate the total sum. Just add the difference between current value and
			 * the element at the current index in the ring buffer to the total sum. */
			const float previous_fps_at_this_index = last_N_fps_values[ rolling_avg_index ];
			last_N_fps_values_sum += fps - previous_fps_at_this_index;

			const float previous_frame_time_at_this_index = last_N_frame_times[ rolling_avg_index ];
			last_N_frame_times_sum += frame_time - previous_frame_time_at_this_index;

			last_N_fps_values[ rolling_avg_index ] = fps;
			last_N_frame_times[ rolling_avg_index++ ] = frame_time;

			rolling_avg_index = rolling_avg_index % ROLLING_AVG_FPS_FRAME_COUNT;

			rolling_avg_fps        = ( u16 )Math::Round( last_N_fps_values_sum / ROLLING_AVG_FPS_FRAME_COUNT );
			rolling_avg_frame_time = last_N_frame_times_sum / ROLLING_AVG_FPS_FRAME_COUNT;
		}
	}

	void FrameStatisticsOverlay::Render( Context& editor_context )
	{
		if( ImGuiUtility::BeginOverlay( editor_context.viewport_panel.imgui_window_name.c_str(), ICON_FA_CHART_LINE " Frame Statistics",
										ImGuiUtility::HorizontalPosition::RIGHT, ImGuiUtility::VerticalPosition::TOP,
										&editor_context.show_frame_statistics_overlay ) )
		{
			const auto& style = ImGui::GetStyle();

			const ImVec2 max_size( ImGui::CalcTextSize( "FPS: 999.9 fps  |  # Frames: 99999999" ) + style.ItemInnerSpacing );
			const ImVec2 max_size_half_width( max_size.x / 2.0f, max_size.y );
			const float  max_width = max_size.x;

			ImGui::SetWindowFontScale( 1.2f );
			char text[ 255 ] = {};
			sprintf_s( text,
					   "Avg. FPS:        %hu fps\n"
					   "Avg. Frame Time: %.2f ms\n"
					   "Time:            %.1f s\n"
					   "Frame:           #%-8lld",
					   rolling_avg_fps, rolling_avg_frame_time, editor_context.frame_time.time_since_start, editor_context.frame_time.frame_count );
			local_persist float refresh_rate = Platform::GetMainMonitorRefreshRate();
			ImGui::PushStyleColor( ImGuiCol_PlotLines, Math::ToImVec4( Math::Lerp( Color4::Red(), Color4::Green(), ( float )rolling_avg_fps / refresh_rate ) ) );
			ImGui::PlotLines( "##FPS", last_N_fps_values.data(), ROLLING_AVG_FPS_FRAME_COUNT, rolling_avg_index, text,
							  rolling_avg_fps * 0.9f, rolling_avg_fps * 1.2f, ImVec2{ -1.0f, ImGui::GetTextLineHeight() * 6 } );
			ImGui::PopStyleColor();
			ImGui::SetWindowFontScale( 1.0f );

			/*if( not Math::IsEqual( time_multiplier, 1.0f ) )
			{
				ImGui::SameLine();
				ImGui::TextColored( ImGui::GetStyleColorVec4( ImGuiCol_HeaderActive ), " (%.3f ms)", time_delta * 1000.0f );
			}*/

			ImGui::SetNextItemWidth( max_width - ImGui::CalcTextSize( "Time Multiplier" ).x );
			ImGui::SliderFloat( "Time Multiplier", &editor_context.frame_time.time_multiplier, 0.01f, 5.0f, "x %.2f", ImGuiSliderFlags_Logarithmic );

			if( !editor_context.frame_time.TimeIsFrozen() && ImGui::Button( ICON_FA_PAUSE " Pause", max_size_half_width ) )
				editor_context.frame_time.FreezeTime();
			else if( editor_context.frame_time.TimeIsFrozen() && ImGui::Button( ICON_FA_PLAY " Resume", max_size_half_width ) )
				editor_context.frame_time.UnfreezeTime();
			ImGui::SameLine();
			if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##time_multiplier", max_size_half_width ) )
				editor_context.frame_time.time_multiplier = 1.0f;
		}

		ImGuiUtility::EndOverlay();
	}
}
