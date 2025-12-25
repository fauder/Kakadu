// Engine Includes.
#include "GLLogger.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Engine
{
	GLLogger::GLLogGroup::GLLogGroup( GLLogger* logger, const char* group_name )
		:
		logger( logger )
	{
		logger->PushGroup( group_name );
	}

	GLLogger::GLLogGroup::GLLogGroup( GLLogGroup&& donor )
		:
		logger( std::exchange( donor.logger, nullptr ) )
	{}

	GLLogger::GLLogGroup& GLLogger::GLLogGroup::operator=( GLLogGroup&& donor )
	{
		logger = std::exchange( donor.logger, nullptr );
		return *this;
	}

	GLLogger::GLLogGroup::~GLLogGroup()
	{
		Close();
	}

	void GLLogger::GLLogGroup::Close()
	{
		if( logger )
		{
			logger->PopGroup();
			logger = nullptr;
		}
	}

	GLLogger::GLLogger()
		:
		logger(
		{
			/* Red for errors.			*/ ImVec4( 1.0f,  0.0f,  0.0f,  1.0f ), // <-- ERROR
			/* Yellow for warnings.		*/ ImVec4( 1.0f,  1.0f,  0.0f,  1.0f ), // <-- WARNING
			/* Dark green for markers.	*/ ImVec4( 0.04f, 0.3f,  0.15f, 1.0f ), // <-- MARKER
			/* Teal for group logs.		*/ ImVec4( 0.38f, 1.0f,  0.82f, 1.0f ), // <-- PUSH_GROUP
			/* Teal for group logs.		*/ ImVec4( 0.38f, 1.0f,  0.82f, 1.0f ), // <-- POP_GROUP
			/* White for normal logs.	*/ ImVec4( 1.0f,  1.0f,  1.0f,  1.0f ), // <-- NORMAL
		} )
	{
	}
		
	GLLogger::~GLLogger()
	{
	}

	void GLLogger::Info( const std::string& message )
	{
		logger.AddLog( GLLogType::NORMAL, message );
	}

	void GLLogger::Info( const char* message )
	{
		logger.AddLog( GLLogType::NORMAL, message );
	}

	void GLLogger::Warning( const std::string& message )
	{
		logger.AddLog( GLLogType::WARNING, ICON_FA_TRIANGLE_EXCLAMATION + ( " " + message ) );
	}

	void GLLogger::Warning( const char* message )
	{
		logger.AddLog( GLLogType::WARNING, ( std::string( ICON_FA_TRIANGLE_EXCLAMATION ) + " " + message ).c_str() );
	}

	void GLLogger::Error( const std::string& message )
	{
		logger.AddLog( GLLogType::ERROR_, ICON_FA_CIRCLE_EXCLAMATION + ( " " + message ) );
	}

	void GLLogger::Error( const char* message )
	{
		logger.AddLog( GLLogType::ERROR_, ( std::string( ICON_FA_CIRCLE_EXCLAMATION ) + " " + message ).c_str() );
	}

	/* omit_empty_group: If true, defers the push operation until an actual log is recorded between this function call & the PopGroup() call. If no calls were made in-between,
	 * the group is not pushed/popped. It is effectively omitted. */
	void GLLogger::PushGroup( const char* group_name )
	{
		groups_empty.push( group_name ); // This will be checked & popped by the actual debug output callback.

		glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0 /* ignored */, -1, group_name);
	}

	void GLLogger::PopGroup()
	{
		glPopDebugGroup(); // groups_empty will be popped if non-empty, in the actual debug output callback.
	}

	GLLogger::GLLogGroup GLLogger::TemporaryLogGroup( const char* group_name )
	{
		GLLogGroup group( this, group_name );
		return group;
	}

	void GLLogger::Marker( const char* marker_label )
	{
		glDebugMessageInsert( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, marker_label );
	}

	void GLLogger::SetLabel( const GLenum object_type, const GLuint object_id, const char* label ) const
	{
		glObjectLabel( object_type, object_id, -1, label );
	}

	void GLLogger::SetLabel( const GLenum object_type, const GLuint object_id, const std::string& name ) const
	{
		glObjectLabel( object_type, object_id, -1, name.c_str() );
	}

	void GLLogger::GetLabel( const GLenum object_type, const GLuint object_id, char* label ) const
	{
		int label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, label );
	}

	std::string GLLogger::GetLabel( const GLenum object_type, const GLuint object_id ) const
	{
		char OBJECT_LABEL_STORAGE[ GL_MAX_LABEL_LENGTH ];
		int label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, OBJECT_LABEL_STORAGE );
		return OBJECT_LABEL_STORAGE;
	}

	void GLLogger::IgnoreID( const unsigned int id_to_ignore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_ignore, false );
	}

	void GLLogger::DontIgnoreID( const unsigned int id_to_restore )
	{
		glDebugMessageControl( GL_DEBUG_SOURCE_API, GL_DEBUG_TYPE_OTHER, GL_DONT_CARE, 1, &id_to_restore, true );
	}

	void GLLogger::Draw( bool* show )
	{
		logger.Draw( ICON_FA_BOOK " GL Logs", show );
	}

	GLLogger::CallbackType GLLogger::GetCallback()
	{
		return [ = ]( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters )
		{
			this->InternalDebugOutputCallback( source, type, id, severity, length, message, parameters );
		};
	}

/* 
 * Private API:
 */

	void GLLogger::InternalDebugOutputCallback( GLenum source, GLenum type, unsigned int id /* ignored */, GLenum severity, GLsizei length, const char* message, 
												const void* parameters /* ignored */ )
	{
		if( type == GL_DEBUG_TYPE_MARKER )
			return;

		if( not groups_empty.empty() )
		{
			if( type == GL_DEBUG_TYPE_PUSH_GROUP ) // Can not process this type while the group is still empty.
				return;

			if( type == GL_DEBUG_TYPE_POP_GROUP ) // Can not process this type while the group is still empty.
			{
				groups_empty.pop();
				return;
			}

			/* We pop one group name every time a log is made. This way, we can check if any logs were recorded between push/pop of a specific log group & can skip empty groups. */
			auto group_name = groups_empty.top();
			groups_empty.pop();

			/* First log the group name: */
			Log( GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_PUSH_GROUP, GL_DEBUG_SEVERITY_NOTIFICATION, -1, group_name, parameters );

			/* Resume logging the actual message. */
		}

		Log( source, type, severity, length, message, parameters );
	}

	void GLLogger::Log( GLenum source, GLenum type, GLenum severity, GLsizei length, const char* message, const void* parameters )
	{
		constexpr std::size_t fixed_portion_length =
			1 + 1 +				// 1 vertical line + 1 space.
			17 + 1 + 1 + 1 +	// Severity ( max length = 17 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 + 1 +	// Source   ( max length = 13 ) + 1 space + 1 vertical line + 1 space.
			13 + 1 + 1 +		// Type     ( max length = 13 ) + 1 space + 1 vertical line.
			1 + 4;				// Colon + 4 spaces.

		local_persist std::vector< char > full_message( fixed_portion_length + 255, ' ' ); // Initial max. message length of 255 reserved.

		if( length > 255 )
			full_message.resize( fixed_portion_length + length );

		/* Write group names directly. */
		if( type == GL_DEBUG_TYPE_PUSH_GROUP )
			sprintf_s( full_message.data(), full_message.size(), ICON_FA_ANGLE_UP " %s", message );
		else if( type == GL_DEBUG_TYPE_POP_GROUP )
			sprintf_s( full_message.data(), full_message.size(), ICON_FA_ANGLE_DOWN );
		else
		{
			const auto severity_string = GLenumToString_Severity( severity );	// Has a max length of 17.
			const auto source_string   = GLenumToString_Source( source );		// Has a max length of 13.
			const auto type_string     = GLenumToString_Type( type );			// Has a max length of 13.

			sprintf_s( full_message.data(), full_message.size(), ICON_FA_ELLIPSIS_VERTICAL "\t%-17s | %13s | %13s: \t%s", severity_string, source_string, type_string, message );
		}

		logger.AddLog( GLenumToGLLogType( type ), full_message.data() );
	}

	const char* GLLogger::GLenumToString_Source( const GLenum source )
	{
		switch( source )
		{
			case GL_DEBUG_SOURCE_API				: return "GL";
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM 		: return "WINDOW SYS.";
			case GL_DEBUG_SOURCE_SHADER_COMPILER 	: return "SHADER COMP.";
			case GL_DEBUG_SOURCE_THIRD_PARTY 		: return "3RD PARTY";
			case GL_DEBUG_SOURCE_APPLICATION 		: return "APPLICATION";
			case GL_DEBUG_SOURCE_OTHER				: return "OTHER";
			
			default:
				return "INVALID ENUM";
		}
	}

	const char* GLLogger::GLenumToString_Type( const GLenum type )
	{
		
		switch( type )
		{
			case GL_DEBUG_TYPE_ERROR 				: return ICON_FA_CIRCLE_EXCLAMATION;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR 	: return ICON_FA_TRIANGLE_EXCLAMATION " DEPRECATED";
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR 	: return ICON_FA_TRIANGLE_EXCLAMATION " UNDEFINED";
			case GL_DEBUG_TYPE_PORTABILITY 			: return ICON_FA_TRIANGLE_EXCLAMATION " PORTABILITY";
			case GL_DEBUG_TYPE_PERFORMANCE 			: return ICON_FA_GAUGE;
			case GL_DEBUG_TYPE_MARKER 				: return ICON_FA_FLAG;
			case GL_DEBUG_TYPE_PUSH_GROUP 			: return "PUSH_GROUP";
			case GL_DEBUG_TYPE_POP_GROUP 			: return "POP_GROUP";
			case GL_DEBUG_TYPE_OTHER 				: return "OTHER";

			default:
				return "INVALID ENUM";
		}
	}

	const char* GLLogger::GLenumToString_Severity( const GLenum severity )
	{
		switch( severity )
		{
			case GL_DEBUG_SEVERITY_HIGH 			: return "SEVERITY: HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM 			: return "SEVERITY: MEDIUM";
			case GL_DEBUG_SEVERITY_LOW 				: return "SEVERITY: LOW";
			case GL_DEBUG_SEVERITY_NOTIFICATION		: return "SEVERITY: NOTIF.";

			default:
				return "INVALID ENUM";
		}
	}
}