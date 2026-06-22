#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION                    vec3 position;
NORMAL                      vec3 normal;
TEXCOORDS                   vec2 tex_coords;
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM    mat4 world_transform;
INSTANCE_COLOR              vec4 color;
#endif

#pragma feature INSTANCING_ENABLED

#ifdef INSTANCING_ENABLED
out vec4 varying_color;
#else
uniform mat4x4 uniform_transform_world;
#endif

void main()
{
#ifdef INSTANCING_ENABLED
    gl_Position = vec4( position, 1.0 ) * world_transform * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
    varying_color = color;
#else
    gl_Position = vec4( position, 1.0 ) * uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
#endif
}