#version 460 core

#include "_Intrinsic_Other.glsl"

layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 3 ) out;

out vec3 varying_barycentric_coordinates;

void main()
{
    /* Vertex 1: */
    gl_Position = gl_in[ 0 ].gl_Position;
    varying_barycentric_coordinates = vec3( 1.0f, 0.0f, 0.0f );
    EmitVertex();

    /* Vertex 2: */
    gl_Position = gl_in[ 1 ].gl_Position;
    varying_barycentric_coordinates = vec3( 0.0f, 1.0f, 0.0f );
    EmitVertex();

    /* Vertex 3: */
    gl_Position = gl_in[ 2 ].gl_Position;
    varying_barycentric_coordinates = vec3( 0.0f, 0.0f, 1.0f );
    EmitVertex();

    EndPrimitive();
}
