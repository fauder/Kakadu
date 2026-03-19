#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <cstdint>
#include <string>
#include <string_view>

namespace Kakadu::Graphics
{
	struct DeviceInfo
	{
		enum class Vendor : u8
		{
			Unknown,
			Nvidia,
			Amd,
			Intel,
			Mesa
		};

		std::string device_name;
		std::string vendor_name;
		Vendor vendor = Vendor::Unknown;
	};

	DeviceInfo QueryDeviceInfo();
}
