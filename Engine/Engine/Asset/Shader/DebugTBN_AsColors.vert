#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION vec3 position;
NORMAL   vec3 normal;
TANGENT  vec3 tangent;
#ifdef INSTANCING_ENABLED
INSTANCE_WORLD_TRANSFORM mat4 world_transform;
#endif

out VS_Out
{
    vec4 varying_tangent_world_space;
    vec4 varying_bitangent_world_space;
    vec4 varying_normal_world_space;
} vs_out;

#pragma feature INSTANCING_ENABLED

#ifndef INSTANCING_ENABLED
uniform mat4x4 uniform_transform_world;
#endif

void main()
{
#ifndef INSTANCING_ENABLED
    mat4x4 world_transform = uniform_transform_world;
#endif

    mat3x3 world_transform_for_normals = mat3x3( transpose( inverse( world_transform ) ) );

    vs_out.varying_tangent_world_space   = vec4( normalize( tangent * mat3x3( world_transform ) ), 0.0 );
    vs_out.varying_normal_world_space    = vec4( normalize( normal * world_transform_for_normals ), 0.0 );
    vs_out.varying_bitangent_world_space = vec4( normalize( cross( vs_out.varying_tangent_world_space.xyz, vs_out.varying_normal_world_space.xyz ) ), 0.0 );
    
    gl_Position = vec4( position, 1.0 ) * world_transform * _INTRINSIC_TRANSFORM_VIEW_PROJECTION;
}