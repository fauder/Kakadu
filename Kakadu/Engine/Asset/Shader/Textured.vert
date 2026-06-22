#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION  vec3 position;
TEXCOORDS vec2 tex_coords;

out vec2 varying_tex_coords;

#pragma driven
uniform mat4x4 uniform_transform_world;
uniform vec4 uniform_texture_scale_and_offset = vec4( 1, 1, 0, 0 );

void main()
{
    gl_Position = vec4( position, 1.0 ) * uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;

    varying_tex_coords = tex_coords * uniform_texture_scale_and_offset.xy + uniform_texture_scale_and_offset.zw;
}