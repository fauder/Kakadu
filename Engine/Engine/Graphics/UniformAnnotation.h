#pragma once

// std Includes.
#include <string>

namespace Engine::UniformAnnotation
{
	enum class Type
	{
		Unassigned, // This is necessary to be able to have a None value & to be able to check whether there is a mismatch between multiple stages of the shader for a given parameter.

		None,
		AsColor3,
		AsColor4,
		AsArray,
		AsSlider_In_Pixels,
		AsSlider_Normalized,
		AsSlider_Normalized_Logarithmic,
		AsSlider_Normalized_Percentage,
		AsSlider_Normalized_Percentage_Logarithmic
	};

	static Type Type_StringToEnum( const std::string& string )
	{
		if( string == "color3" )
			return Type::AsColor3;
		else if( string == "color4" )
			return Type::AsColor4;
		else if( string.compare( 0, 5, "array", 5 ) == 0 )
			return Type::AsArray;
		else if( string == "slider_in_pixels" )
			return Type::AsSlider_In_Pixels;
		else if( string == "normalized" )
			return Type::AsSlider_Normalized;
		else if( string == "normalized_logarithmic" )
			return Type::AsSlider_Normalized_Logarithmic;
		else if( string == "normalized_percentage" )
			return Type::AsSlider_Normalized_Percentage;
		else if( string == "normalized_percentage_logarithmic" )
			return Type::AsSlider_Normalized_Percentage_Logarithmic;
		else
			return Type::None;
	}
}
