// Engine Includes.
#include "Core/Console.h"
#include "Graphics/RHI/RHI.h"

namespace Kakadu::Editor::LoggingTest
{
	void RunUpdateLogging( Console& console )
	{
		for( size_t i = 0; i < 10; i++ )
		{
			console.Log( "[On Update()] Frame logging tick." );
			console.Log( "[On Update()] Extended message: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." );
			console.Log( "[On Update()] Repetitive patterns: alpha beta gamma delta alpha beta gamma delta alpha beta gamma delta alpha beta gamma delta alpha beta gamma delta alpha beta gamma delta." );
		}
	}

	void RunInitializationLogging( Console& console )
	{
		glEnable( GL_INVALID_ENUM ); // Emit an error.

		GLenum sources[] =
		{
			GL_DEBUG_SOURCE_APPLICATION,
			GL_DEBUG_SOURCE_THIRD_PARTY,
			// Other types can not be emitted on demand by the application per spec.
		};

		GLenum types[] =
		{
			GL_DEBUG_TYPE_ERROR,
			GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR,
			GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR,
			GL_DEBUG_TYPE_PORTABILITY,
			GL_DEBUG_TYPE_PERFORMANCE,
			GL_DEBUG_TYPE_MARKER,
			GL_DEBUG_TYPE_PUSH_GROUP,
			GL_DEBUG_TYPE_POP_GROUP,
			GL_DEBUG_TYPE_OTHER
		};

		GLenum severities[] =
		{
			GL_DEBUG_SEVERITY_HIGH,
			GL_DEBUG_SEVERITY_MEDIUM,
			GL_DEBUG_SEVERITY_LOW, // Not reported on AMD AFAIK.
			GL_DEBUG_SEVERITY_NOTIFICATION
		};

		int i = 1;
		for( auto s : sources )
		{
			for( auto t : types )
			{
				for( auto sev : severities )
				{
					std::string msg = "OpenGL debug message #" + std::to_string( i++ ) + " (test injection).";
					glDebugMessageInsert( s, t, 0, sev, -1, msg.c_str() );

					console.Log( "Standard log entry." );
					console.Log( "Extended message: Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua." );
					console.Log( "Secondary log entry." );

					console.LogWarning( "Example warning condition triggered for validation." );
					console.LogError( "Example error condition triggered for validation." );
					console.LogSuccess( "Example success state reported." );
				}
			}
		}
	}
}