#version 460 core

#include "_Intrinsic_Other.glsl"

layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 3 ) out;

/* Strategy: Convert bary coords (which are trivially defined as <1,0,0>, <0,1,0> & <0,0,1> for the triangle vertices/corners) to edge distances in screen-space pixels by:
 * Vertex pos. in clip space -> To NDC -> To pixels in screen space -> Calculate the distance from the edge midpoint to the vertex in screen space -> Multiply with bary coords for that vertex.
 *
 * Since the clip space is not Euclidian, geometric operations such as taking the midpoint do not make sense there semantically (and yields numerically incorrect results). */
noperspective out vec3 varying_edge_distances_in_pixels; // Also make sure to disable hyperbolic interpolation as we directly calculated the screen-space values anyway.

void main()
{
    /*                                              Divide by w to convert to NDC               Scale & Translate the origin    Scale by the screen resolution. */
    vec2 vertex_0_in_screen_space = ( ( gl_in[ 0 ].gl_Position.xy / gl_in[ 0 ].gl_Position.w )      * 0.5 + 0.5 )               * _INTRINSIC_VIEWPORT_SIZE.xy;
    vec2 vertex_1_in_screen_space = ( ( gl_in[ 1 ].gl_Position.xy / gl_in[ 1 ].gl_Position.w )      * 0.5 + 0.5 )               * _INTRINSIC_VIEWPORT_SIZE.xy;
    vec2 vertex_2_in_screen_space = ( ( gl_in[ 2 ].gl_Position.xy / gl_in[ 2 ].gl_Position.w )      * 0.5 + 0.5 )               * _INTRINSIC_VIEWPORT_SIZE.xy;

    /* Vertex 1: */
    gl_Position = gl_in[ 0 ].gl_Position;

    varying_edge_distances_in_pixels = vec3( length( ( vertex_1_in_screen_space + vertex_2_in_screen_space ) * 0.5 - vertex_0_in_screen_space ), 0.0f, 0.0f );

    EmitVertex();

    /* Vertex 2: */
    gl_Position = gl_in[ 1 ].gl_Position;

    varying_edge_distances_in_pixels = vec3( 0.0f, length( ( vertex_0_in_screen_space + vertex_2_in_screen_space ) * 0.5 - vertex_1_in_screen_space ), 0.0f );

    EmitVertex();

    /* Vertex 3: */
    gl_Position = gl_in[ 2 ].gl_Position;

    varying_edge_distances_in_pixels = vec3( 0.0f, 0.0f, length( ( vertex_0_in_screen_space + vertex_1_in_screen_space ) * 0.5 - vertex_2_in_screen_space ) );

    EmitVertex();

    EndPrimitive();
}
