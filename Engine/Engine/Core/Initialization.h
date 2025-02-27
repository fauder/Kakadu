#pragma once

namespace Engine
{
	namespace Initialization
	{
		struct UniformInitialization	{};
		struct ZeroInitialization		{};
		struct NoInitialization			{};
	}

	inline constexpr Initialization::UniformInitialization	UNIFORM_INITIALIZATION;
	inline constexpr Initialization::ZeroInitialization		ZERO_INITIALIZATION;
	inline constexpr Initialization::NoInitialization		NO_INITIALIZATION;
}