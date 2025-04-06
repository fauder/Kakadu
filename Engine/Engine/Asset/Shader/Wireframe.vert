#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION                    vec3 position;
#ifdef OFFSET_DEPTH
NORMAL                      vec3 normal;
#endif
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM    mat4 world_transform;
#endif

#pragma feature INSTANCING_ENABLED
#pragma feature OFFSET_DEPTH

#ifdef INSTANCING_ENABLED
#define WORLD_TRANSFORM world_transform
#else
uniform mat4 uniform_transform_world;
#define WORLD_TRANSFORM uniform_transform_world
#endif

void main()
{
    /* Vertex shader can directly transform to clip space because geometry shader will work with clip space positions. */

    gl_Position = vec4( position, 1.0 ) * WORLD_TRANSFORM * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
}