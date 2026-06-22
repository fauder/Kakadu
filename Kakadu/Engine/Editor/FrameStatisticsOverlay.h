#pragma once

// Engine Includes.
#include "EditorContext.h"

namespace Kakadu::Editor
{
	struct FrameStatisticsOverlay
	{
		void Update( const float time_delta_real );
		void Render( Context& context );

		/* Since only 1 value in the ring buffer changes every frame, no need to re-calculate the total sum. Just add the difference between current value and
		 * the element at the current index in the ring buffer to the total sum. */
		float last_N_fps_values_sum  = 0.0f;
		float last_N_frame_times_sum = 0.0f;

		u16 rolling_avg_fps = 0;
		/* 2 bytes of padding. */
		i32 rolling_avg_index = 0;

		float rolling_avg_frame_time = 0.0f;

		static constexpr u8 ROLLING_AVG_FPS_FRAME_COUNT = 144;
		std::array< float, ROLLING_AVG_FPS_FRAME_COUNT > last_N_fps_values;
		std::array< float, ROLLING_AVG_FPS_FRAME_COUNT > last_N_frame_times;
	};
}
