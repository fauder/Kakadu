#version 460 core

#include "_Intrinsic_Other.glsl"

layout ( triangles ) in;
layout ( line_strip, max_vertices = 12 ) out;

in VS_Out
{
    vec4 varying_normal_view_space;
    vec4 varying_tangent_view_space;
} gs_in[];

out vec3 varying_vertex_color;

const float LINE_SCALE_DEFAULT  = 0.3f;
const float LINE_OFFSET_DEFAULT = 0.02f;

uniform float uniform_line_scale_override;
uniform float uniform_line_offset_from_surface_override;

const vec3 NORMAL_COLOR  = vec3( 0.5f, 0.5f, 1.0f );
const vec3 TANGENT_COLOR = vec3( 1.0f, 0.5f, 0.5f );

void GenerateVertexNormalLine( int vertex_index )
{
    const float line_scale  = uniform_line_scale_override               == 0.0f ? LINE_SCALE_DEFAULT    : uniform_line_scale_override;
    const float line_offset = uniform_line_offset_from_surface_override == 0.0f ? LINE_OFFSET_DEFAULT   : uniform_line_offset_from_surface_override;

    const float line_total_length = line_scale + line_offset;

    const vec4 normal               = gs_in[ vertex_index ].varying_normal_view_space;
    const vec4 vertex_position      = gl_in[ vertex_index ].gl_Position + normal * line_offset;
    const vec4 vertex_position_clip = vertex_position * _INTRINSIC_TRANSFORM_PROJECTION;

    /* Normal Line: */
    gl_Position = vertex_position_clip;
    varying_vertex_color = NORMAL_COLOR;
    EmitVertex();

    gl_Position += ( vertex_position + normal * line_total_length ) * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = NORMAL_COLOR;
    EmitVertex();

    EndPrimitive();

    /* Tangent Line: */
    const vec4 tangent = gs_in[ vertex_index ].varying_tangent_view_space;

    gl_Position = vertex_position_clip;
    varying_vertex_color = TANGENT_COLOR;
    EmitVertex();

    gl_Position += ( vertex_position + tangent * line_total_length ) * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = TANGENT_COLOR;
    EmitVertex();

    EndPrimitive();
}

void main()
{
   GenerateVertexNormalLine( 0 );
   GenerateVertexNormalLine( 1 );
   GenerateVertexNormalLine( 2 );
}  