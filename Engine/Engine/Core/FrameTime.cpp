// Engine Includes.
#include "FrameTime.h"
#include "Platform.h"
#include "Math/TypeTraits.h"

namespace Engine
{
	void FrameTime::Update()
	{
		frame_count++;

		time_since_start = Platform::CurrentTime();

		time_delta_real = time_since_start - time_previous_since_start;

		time_current += time_delta_real * time_multiplier;

		time_delta = time_current - time_previous;

		time_previous             = time_current;
		time_previous_since_start = time_since_start;

		time_sin      = Math::Sin( Radians( time_current ) );
		time_cos      = Math::Cos( Radians( time_current ) );
		time_mod_1    = std::fmod( time_current, 1.0f );
		time_mod_2_pi = std::fmod( time_current, Constants< float >::Two_Pi() );
	}

	void FrameTime::FreezeTime()
	{
		time_multiplier = 0.0f;
	}

	void FrameTime::UnfreezeTime()
	{
		time_multiplier = 1.0f;
	}

	bool FrameTime::TimeIsFrozen()
	{
		return Math::IsZero( time_multiplier );
	}
}
