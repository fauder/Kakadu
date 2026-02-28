#pragma once

namespace Kakadu
{
	struct FrameTime
	{
		void Update();

		void FreezeTime();
		void UnfreezeTime();
		bool TimeIsFrozen();

		long long frame_count = 1;

		float time_multiplier = 1.0f;

		float time_delta;
		float time_delta_real;

		float time_current;

		float time_previous = 0.0f;
		float time_previous_since_start = 0.0f;

		float time_since_start;

		/* Derived info.: */

		float time_sin;
		float time_cos;
		float time_mod_1;
		float time_mod_2_pi;

		/* 4 bytes of padding. */
	};
}
