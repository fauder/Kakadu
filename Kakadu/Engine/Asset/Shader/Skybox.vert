#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION vec3 position;

out vec3 varying_tex_coords;

void main()
{
    varying_tex_coords = position;

    gl_Position = vec4( position, 1.0f ) * _INTRINSIC_TRANSFORM_VIEW_ROTATION_ONLY * _INTRINSIC_TRANSFORM_PROJECTION;
    gl_Position.z = gl_Position.w;
}
