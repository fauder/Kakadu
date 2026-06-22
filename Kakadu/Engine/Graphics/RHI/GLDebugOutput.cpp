// Engine Includes.
#include "RHI.h"
#include "GLDebugOutput.h"
#include "Core/Assertion.h"
#include "Core/LogType.h"
#include "Core/LogSink.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <iostream>

namespace Kakadu::RHI
{
/*
 * Internal Functions:
 */

	internal_function const char* ConvertGLMessageSourceString( const u32 source )
	{
		switch( source )
		{
			case GL_DEBUG_SOURCE_API				: return "OPENGL";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM 		: return "WND SYS";
			case GL_DEBUG_SOURCE_SHADER_COMPILER 	: return "SHADER COMPILER";
			case GL_DEBUG_SOURCE_THIRD_PARTY 		: return "3RD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION 		: return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER				: return "OTHER";
			
			default:
				return "INVLDENUM";
		}
	}

	internal_function const char* ConvertGLMessageTypeString( const u32 type )
	{
		
		switch( type )
		{
			case GL_DEBUG_TYPE_ERROR 				: return "ERROR";
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 	: return "DEPRECATED";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 	: return "UNDEFINED";
			case GL_DEBUG_TYPE_PORTABILITY 			: return "PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE 			: return "PERFORMANCE";
			case GL_DEBUG_TYPE_MARKER 				: return "MARKER";
			case GL_DEBUG_TYPE_PUSH_GROUP 			: return "PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP 			: return "POP_GROUP";
			case GL_DEBUG_TYPE_OTHER 				: return "OTHER";
			default									: return "INVALIDENUM";
		}
	}

	internal_function const char* ConvertGLMessageSeverityString( const u32 severity )
	{
		switch( severity )
		{
			case GL_DEBUG_SEVERITY_HIGH 			: return "SEVERE";
			case GL_DEBUG_SEVERITY_MEDIUM 			: return "MEDIUM";
			case GL_DEBUG_SEVERITY_LOW 				: return "MINOR";
			case GL_DEBUG_SEVERITY_NOTIFICATION		: return "NOTICE";

			default:
				return "INVALID ENUM";
		}
	}

	internal_function Log::Type ConvertGLMessageType( u32 value )
	{
		switch( value )
		{
			case GL_DEBUG_TYPE_ERROR:				return Log::Type::ERROR_;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return Log::Type::WARNING;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:	return Log::Type::WARNING;
			case GL_DEBUG_TYPE_PORTABILITY:			return Log::Type::WARNING;
			case GL_DEBUG_TYPE_PERFORMANCE:			return Log::Type::WARNING;

			case GL_DEBUG_TYPE_MARKER:				return Log::Type::GROUP_SEPARATOR;
			case GL_DEBUG_TYPE_PUSH_GROUP:			return Log::Type::GROUP_SEPARATOR;
			case GL_DEBUG_TYPE_POP_GROUP:			return Log::Type::GROUP_SEPARATOR;

			case GL_DEBUG_TYPE_OTHER:				return Log::Type::NORMAL;

			default:
				std::cerr << "Invalid GL enum value passed to ConvertGLMessageType( u32 value )!\n";
				return Log::Type::INVALID;
		}
	}

	internal_function void LogDebugOutput( u32 source, u32 type, u32 severity, i32 length, const char* message, const void* parameters )
	{
		constexpr std::size_t fixed_portion_length =
			6 + 1 + 1 + 1 +	// Source ( max length = 6  ) + 1 space + 1 vertical line + 1 space.
			11 + 1 + 1;		// Type   ( max length = 11 ) + 1 colon + 1 space.

		char buffer[ fixed_portion_length + 255 ];
		ASSERT_EDITOR_ONLY( length <= 255 );

		const auto source_string = ConvertGLMessageSourceString( source ); // Has a max length > 6 BUT most of those are rare.
		const auto type_string = ConvertGLMessageTypeString( type ); // Has a max length of 11.

		sprintf_s( buffer, sizeof( buffer ), "%-6s | %s: %s", source_string, type_string, message ); // Ignore severity; not that useful.
		Log::Sink::Dispatch( ConvertGLMessageType( type ), buffer );
	}

	internal_function void InternalDebugOutputCallback( u32 source, u32 type, u32 id /* ignored */, u32 severity, i32 length, const char* message,
														const void* parameters /* ignored */ )
	{
		if( type == GL_DEBUG_TYPE_PUSH_GROUP ||
			type == GL_DEBUG_TYPE_POP_GROUP )
			return;

		LogDebugOutput( source, type, severity, length, message, parameters );
	}

/*
 * Marker:
 */

	void GLDebugOutput::Marker( const char* marker_label )
	{
		glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, marker_label );
	}

/*
 * Filtering IDs:
 */

	void GLDebugOutput::IgnoreID( const u32 id_to_ignore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_ignore, false );
	}

	void GLDebugOutput::DontIgnoreID( const u32 id_to_restore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_restore, true );
	}

/*
 * Queries:
 */

	GLDebugOutput::CallbackType GLDebugOutput::GetCallback()
	{
		return [ = ]( u32 source, u32 type, u32 id, u32 severity, i32 length, const char* message, const void* parameters )
		{
			InternalDebugOutputCallback( source, type, id, severity, length, message, parameters );
		};
	}
}