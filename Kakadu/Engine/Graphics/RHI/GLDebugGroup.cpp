// Engine Includes.
#include "RHI.h"
#include "GLDebugGroup.h"

namespace Kakadu::RHI
{
    GLDebugGroup::GLDebugGroup( std::string_view name )
    {
        glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, name.data() );
    }

    GLDebugGroup::~GLDebugGroup()
    {
        glPopDebugGroup();
    }
}