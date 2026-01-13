// Engine Includes.
#include "Graphics.h"
#include "GraphicsDeviceInfo.h"
#include "Core/Macros.h"
#include "Core/Utility.hpp"

namespace Engine::Graphics
{
	internal_function DeviceInfo::Vendor DetectVendor( const std::string_view vendor_string )
	{
		const std::string v = Utility::String::ToLowerAscii( vendor_string );

		if( v.find( "nvidia" ) != std::string::npos )
			return DeviceInfo::Vendor::Nvidia;

		if( v.find( "amd" ) != std::string::npos ||
			v.find( "advanced micro devices" ) != std::string::npos ||
			v.find( "ati" ) != std::string::npos )
			return DeviceInfo::Vendor::Amd;

		if( v.find( "intel" ) != std::string::npos )
			return DeviceInfo::Vendor::Intel;

		if( v.find( "mesa" ) != std::string::npos )
			return DeviceInfo::Vendor::Mesa;

		return DeviceInfo::Vendor::Unknown;
	}

	DeviceInfo QueryDeviceInfo()
	{
		DeviceInfo info{};

		const char* vendor_cstr =
			reinterpret_cast< const char* >( glGetString( GL_VENDOR ) );

		const char* renderer_cstr =
			reinterpret_cast< const char* >( glGetString( GL_RENDERER ) );

		if( vendor_cstr )
		{
			info.vendor_name = vendor_cstr;
			info.vendor = DetectVendor( info.vendor_name );
		}

		if( renderer_cstr )
		{
			info.device_name = renderer_cstr;
		}

		return info;
	}
}
