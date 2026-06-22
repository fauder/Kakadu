#pragma once

// Engine Includes.
#include "Core/Macros.h"
#include "Core/Types.h"

// std Includes.
#include <functional>

namespace Kakadu::RHI::GLDebugOutput
{
	using CallbackType = std::function< void( u32 source, u32 type, u32 id /* will be ignored */, u32 severity, i32 length, const char* message,
											  const void* parameters /* will be ignored */ ) >;

/* Marker: */
	void Marker( const char* marker_label );

/* Filtering IDs: */
	void IgnoreID( const u32 id_to_ignore );
	void DontIgnoreID( const u32 id_to_restore );

/* Queries: */
	CallbackType GetCallback();
};
