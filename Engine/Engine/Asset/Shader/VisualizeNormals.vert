#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION vec3 position;
NORMAL   vec3 normal;
TANGENT  vec3 tangent;

out VS_Out
{
    vec4 varying_normal_view_space;
    vec4 varying_tangent_view_space;
} vs_out;

uniform mat4x4 uniform_transform_world;

void main()
{
    mat4x4 world_view_transform             = uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW;
    mat3x3 world_view_transform_for_normals = mat3x3( transpose( inverse( world_view_transform ) ) );

    vs_out.varying_normal_view_space = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );

    vs_out.varying_tangent_view_space = vec4( normalize( tangent * mat3x3( world_view_transform ) ), 0.0 );
    
    gl_Position = vec4( position, 1.0 ) * world_view_transform;
}