// Engine Includes.
#include "RHI.h"
#include "DebugLabel.h"
#include "Core/Macros.h"

namespace Kakadu::RHI::DebugLabel
{
	void Set( const u32 object_type, const u32 object_id, const char* label )
	{
		glObjectLabel( object_type, object_id, -1, label );
	}

	void Set( const u32 object_type, const u32 object_id, const std::string& name )
	{
		glObjectLabel( object_type, object_id, -1, name.c_str() );
	}

	void Get( const u32 object_type, const u32 object_id, char* label )
	{
		i32 label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, label );
	}

	std::string Get( const u32 object_type, const u32 object_id )
	{
		thread_local_persist char OBJECT_LABEL_STORAGE[ GL_MAX_LABEL_LENGTH ];
		i32 label_length;
		glGetObjectLabel( object_type, object_id, GL_MAX_LABEL_LENGTH, &label_length, OBJECT_LABEL_STORAGE );
		return OBJECT_LABEL_STORAGE;
	}
}
