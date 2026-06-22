#pragma once

// Engine Includes.
#include "Core/Macros.h"

// std Includes.
#include <string_view>

#define KAKADU_GL_DEBUG_GROUP( name ) Kakadu::RHI::GLDebugGroup kakadu_gl_debug_group_object( name );

namespace Kakadu::RHI
{
    struct GLDebugGroup
    {
        GLDebugGroup( std::string_view name );
        ~GLDebugGroup();

        DELETE_COPY_CONSTRUCTORS( GLDebugGroup );
        DEFAULT_MOVE_CONSTRUCTORS( GLDebugGroup );
    };
}
