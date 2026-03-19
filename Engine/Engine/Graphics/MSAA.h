#pragma once

// std Includes.
#include <cstdint>

namespace Kakadu
{
	struct MSAA
	{
		bool IsEnabled() const { return sample_count > 1; }

		u8 sample_count = 1;
	};
}
