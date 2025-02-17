#pragma once

// std Includes.
#include <cstdint>

namespace Engine
{
	struct MSAA
	{
		inline bool IsEnabled() const { return sample_count > 1; }

		std::uint8_t sample_count = 1;
	};
}
