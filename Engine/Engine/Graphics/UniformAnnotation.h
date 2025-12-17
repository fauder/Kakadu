#pragma once

// std Includes.
#include <string>

namespace Engine::UniformAnnotation
{
	enum class Type : std::uint8_t
	{
		/* A uniform may be defined for a shader stage but not defined for another stage, in the same shader program.
		 * Therefore, to be able to detect mismatches between stages correctly, we need a dedicated 'Unassigned' enumeration here.
		 * Otherwise, if we used 'None' for this, a uniform existing only in the fragment shader would report its type and the vertex shader would report it as None, causing a mismatch.
		 * while in reality, this is a perfectly fine scenario that shouldn't need any attention. */
		Unassigned,

		None,
		Color3,
		Color4,
		Array,
		Slider,
		Driven,
	};

	enum class SliderFlags : std::uint8_t
	{
		None                = 0,
		Logarithmic         = 1 << 0,
		DisplayAsPercentage = 1 << 1 // Expects value in normalized [0, 1] range. Displays % instead [0, 100] (via remapping) but keeps the value as is under the hood.

		// The rest are reserved.
	};

	inline Type StringToType( const std::string_view string )
	{
		if( string == "color3" )
			return Type::Color3;
		else if( string == "color4" )
			return Type::Color4;
		else if( string == "array" )
			return Type::Array;
		else if( string == "slider" )
			return Type::Slider;
		else if( string == "driven" )
			return Type::Driven;
		else
			return Type::None;
	}
	
	inline SliderFlags StringToSliderFlags( const std::string_view string )
	{
		if( string == "logarithmic" )
			return SliderFlags::Logarithmic;
		else if( string.find( "percent" ) != std::string::npos )
			return SliderFlags::DisplayAsPercentage;
		else
			return SliderFlags::None;
	}
}
