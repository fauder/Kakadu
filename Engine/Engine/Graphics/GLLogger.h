#pragma once

// Engine Includes.
#include "RHI/DebugMessageType.h"
#include "Core/Macros.h"
#include "Core/ImGuiLog.hpp"
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
		GLLogger();

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( GLLogger );

		~GLLogger();

	/* Custom messages: */
		void Info( const std::string& message );
		void Info( const char* message );
		void Warning( const std::string& message );
		void Warning( const char* message );
		void Error( const std::string& message );
		void Error( const char* message );

	/* Grouping: */
		/* omit_empty_group: If true, defers the push operation until an actual log is recorded between this function call & the PopGroup() call. If no calls were made in-between,
		 * the group is not pushed/popped. It is effectively omitted. */
		void PushGroup( const char* group_name );
		void PopGroup();

		GLLogGroup TemporaryLogGroup( const char* group_name );

	/* Marker: */
		void Marker( const char* marker_label );

	/* Labeling: */
		void SetLabel( const u32 object_type, const u32 object_id, const char* label ) const;
		void SetLabel( const u32 object_type, const u32 object_id, const std::string& name ) const;
		void GetLabel( const u32 object_type, const u32 object_id, char* label ) const;
		std::string GetLabel( const u32 object_type, const u32 object_id ) const;

	/* Filtering IDs: */
		static void IgnoreID( const u32 id_to_ignore );
		static void DontIgnoreID( const u32 id_to_restore );

	/* Main: */
		void Draw( bool* show = nullptr );

	/* Queries: */
		CallbackType GetCallback();

	private:
		void InternalDebugOutputCallback( u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* parameters );
		void Log( u32 source, u32 type, u32 severity, i32 length, const char* message, const void* parameters );

		static const char* GLenumToString_Source( const u32 source );
		static const char* GLenumToString_Type( const u32 type );
		static const char* GLenumToString_Severity( const u32 severity );

	private:
		ImGuiLog< RHI::DebugMessageType, std::size_t( RHI::DebugMessageType::COUNT ) > logger;

		std::stack< const char* > groups_empty; /* We keep tabs of the empty groups & while we do push/pop them, we do not log them. */
	};
}
