#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"
#include "_Intrinsic_Other.glsl"

POSITION    vec3 position;
NORMAL      vec3 normal;
TEXCOORDS   vec2 tex_coords;

out vec4 varying_position_view_space;
out vec4 varying_normal_view_space;
out vec2 varying_tex_coords;

uniform mat4x4 uniform_transform_world;
uniform vec4 uniform_texture_scale_and_offset;

#pragma slider( 0, 5, logarithmic )
uniform float uniform_outline_thickness;

void main()
{
    float final_model_scale = 1.0f + uniform_outline_thickness;
    mat4x4 outline_thickness_scale_transform = mat4x4( vec4( final_model_scale, vec3( 0 ) ),
                                                       vec4( 0, final_model_scale, vec2( 0 ) ),
                                                       vec4( 0, 0, final_model_scale, 0 ),
                                                       vec4( 0, 0, 0, 1 ) );
    mat4x4 world_view_transform             = outline_thickness_scale_transform * uniform_transform_world * _INTRINSIC_TRANSFORM_VIEW;
    mat3x3 world_view_transform_for_normals = mat3x3( transpose( inverse( world_view_transform ) ) );

    varying_position_view_space = vec4( position, 1.0 ) * world_view_transform;
    varying_normal_view_space   = vec4( normalize( normal * world_view_transform_for_normals ), 0.0 );
    varying_tex_coords          = tex_coords * uniform_texture_scale_and_offset.xy + uniform_texture_scale_and_offset.zw;
    
    gl_Position = varying_position_view_space * _INTRINSIC_TRANSFORM_PROJECTION;
}