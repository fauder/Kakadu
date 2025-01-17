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

const float MAGNITUDE = 0.4;

void GenerateVertexNormalLine( int vertex_index )
{
    /* Normal Line: */
    gl_Position = gl_in[ vertex_index ].gl_Position * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = vec3( 0.0f, 0.0f, 1.0f );
    EmitVertex();

    gl_Position += vec4( gl_in[ vertex_index ].gl_Position + gs_in[ vertex_index ].varying_normal_view_space * MAGNITUDE ) * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = vec3( 0.0f, 0.0f, 1.0f );
    EmitVertex();

    EndPrimitive();

    /* Tangent Line: */
    gl_Position = gl_in[ vertex_index ].gl_Position * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = vec3( 1.0f, 0.0f, 0.0f );
    EmitVertex();

    gl_Position += vec4( gl_in[ vertex_index ].gl_Position + gs_in[ vertex_index ].varying_tangent_view_space * MAGNITUDE ) * _INTRINSIC_TRANSFORM_PROJECTION;
    varying_vertex_color = vec3( 1.0f, 0.0f, 0.0f );
    EmitVertex();

    EndPrimitive();
}

void main()
{
   GenerateVertexNormalLine( 0 );
   GenerateVertexNormalLine( 1 );
   GenerateVertexNormalLine( 2 );
}  