#pragma once

// Engine Includes.
#include "Core/Types.h"

namespace Kakadu::RHI
{
	struct MSAA
	{
		bool IsEnabled() const { return sample_count > 1; }

		u8 sample_count = 1;
	};
}
