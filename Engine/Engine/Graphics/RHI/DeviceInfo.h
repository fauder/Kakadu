#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <cstdint>
#include <string>

namespace Kakadu::RHI
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
