#pragma once

// Engine Includes.
#include "Core/Console.h"
#include "Core/Macros.h"
#include "Core/Types.h"

// std Includes.
#include <functional>

namespace Kakadu
{
	class GLDebugOutput
	{
	private:
		using CallbackType = std::function< void( u32 source, u32 type, u32 id /* will be ignored */, u32 severity, i32 length, const char* message,
												  const void* parameters /* will be ignored */ ) >;

	public:
		GLDebugOutput( Console& console );

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( GLDebugOutput );

		~GLDebugOutput();

	/* Marker: */
		void Marker( const char* marker_label );

	/* Filtering IDs: */
		static void IgnoreID( const u32 id_to_ignore );
		static void DontIgnoreID( const u32 id_to_restore );

	/* Queries: */
		CallbackType GetCallback();

	private:
		void InternalDebugOutputCallback( u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* parameters );
		void LogDebugOutput( u32 source, u32 type, u32 severity, i32 length, const char* message, const void* parameters );

	private:
		Console& console;
	};
}
