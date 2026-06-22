#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <string>

namespace Kakadu::RHI::DebugLabel
{
	void Set( const u32 object_type, const u32 object_id, const char* label );
	void Set( const u32 object_type, const u32 object_id, const std::string& name );
	void Get( const u32 object_type, const u32 object_id, char* label );
	std::string Get( const u32 object_type, const u32 object_id );
}
