#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION vec3 position;
TEXCOORDS vec2 tex_coords;
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM mat4 world_transform;
#endif

out vec2 varying_tex_coords;

#pragma feature INSTANCING_ENABLED

#ifndef INSTANCING_ENABLED
uniform mat4x4 uniform_transform_world;
#endif

void main()
{
#ifndef INSTANCING_ENABLED
    mat4x4 world_transform = uniform_transform_world;
#endif

    varying_tex_coords = tex_coords;

    gl_Position = vec4( position, 1.0 ) * world_transform * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
}