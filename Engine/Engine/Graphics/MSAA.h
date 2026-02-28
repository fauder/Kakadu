#pragma once

// std Includes.
#include <cstdint>

namespace Kakadu
{
	struct MSAA
	{
		bool IsEnabled() const { return sample_count > 1; }

		std::uint8_t sample_count = 1;
	};
}
