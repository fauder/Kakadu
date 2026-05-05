#pragma once

// Engine Includes.
#include "Core/Macros.h"
#include "Core/ImGuiLogger.h"
#include "Core/Types.h"

// std Includes.
#include <functional>
#include <stack>
#include <string>

namespace Kakadu
{
	class GLLogger
	{
	private:
		struct GLLogGroup
		{
			GLLogGroup( GLLogger* logger, const char* group_name );

			DELETE_COPY_CONSTRUCTORS( GLLogGroup );

			GLLogGroup( GLLogGroup&& donor );
			GLLogGroup& operator=( GLLogGroup&& donor );

			~GLLogGroup();

			void Close();

		private:
			GLLogger* logger;
		};

	private:
		using CallbackType = std::function< void( u32 source, u32 type, u32 id /* will be ignored */, u32 severity, i32 length, const char* message,
												  const void* parameters /* will be ignored */ ) >;

	public:
		GLLogger( ImGuiLogger& logger );

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( GLLogger );

		~GLLogger();

	/* Custom messages: */
		void Info( const std::string& message );
		void Info( const char* message );
		void Warning( const std::string& message );
		void Warning( const char* message );
		void Error( const std::string& message );
		void Error( const char* message );
		void Success( const std::string& message );
		void Success( const char* message );

	/* Grouping: */
		/* omit_empty_group: If true, defers the push operation until an actual log is recorded between this function call & the PopGroup() call. If no calls were made in-between,
		 * the group is not pushed/popped. It is effectively omitted. */
		void PushGroup( const char* group_name );
		void PopGroup();

		GLLogGroup TemporaryLogGroup( const char* group_name );

	/* Marker: */
		void Marker( const char* marker_label );

	/* Filtering IDs: */
		static void IgnoreID( const u32 id_to_ignore );
		static void DontIgnoreID( const u32 id_to_restore );

	/* Queries: */
		CallbackType GetCallback();

	private:
		void InternalDebugOutputCallback( u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* parameters );
		void Log( u32 source, u32 type, u32 severity, i32 length, const char* message, const void* parameters );

	private:
		ImGuiLogger& logger;

		std::stack< const char* > groups_empty; /* We keep tabs of the empty groups & while we do push/pop them, we do not log them. */
	};
}
