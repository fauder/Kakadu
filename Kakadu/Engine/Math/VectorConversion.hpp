#pragma once

// Engine Includes.
#include "Vector.hpp"

// Vendor Includes.
#include "ImGui/imgui.h"

namespace Kakadu::Math
{
	template< std::floating_point Component >
	const ImVec2& ToImVec2( const Vector< Component, 2 >& source )
	{
		return reinterpret_cast< const ImVec2& >( source );
	}

	template< typename Component >
	ImVec2 CopyToImVec2( const Vector< Component, 2 >& source )
	{
		ImVec2 destination;
		destination.x = ( float )source.X();
		destination.y = ( float )source.Y();
		return destination;
	}

	template< std::floating_point Component >
	const ImVec4& ToImVec4( const Vector< Component, 4 >& source )
	{
		return reinterpret_cast< const ImVec4& >( source );
	}

	template< typename Component >
	ImVec4 CopyToImVec4( const Vector< Component, 4 >& source )
	{
		ImVec4 destination;
		destination.x = ( float )source.X();
		destination.y = ( float )source.Y();
		destination.z = ( float )source.Z();
		destination.w = ( float )source.W();
		return destination;
	}
}