#version 460 core

#include "_Intrinsic_Other.glsl"

noperspective in vec3 varying_edge_distances_in_pixels; // Each component holds the distance to an edge of the triangle, in pixels, for the barycentric weights respectively.

out vec4 out_color;

uniform vec4 uniform_color; /* _hint_color4 */
uniform float uniform_line_thickness; /* _hint_slider_in_pixels */

#pragma feature OFFSET_DEPTH
#ifdef OFFSET_DEPTH
#define DEPTH_BIAS 1e-5
#endif

void main()
{
#ifdef OFFSET_DEPTH
    gl_FragDepth = gl_FragCoord.z - DEPTH_BIAS;
#endif

    float closest_edge_distance = min( min( varying_edge_distances_in_pixels.x, varying_edge_distances_in_pixels.y ), varying_edge_distances_in_pixels.z );

    float edge_factor = 1.0f - smoothstep( 0.0f, uniform_line_thickness, closest_edge_distance );

    out_color = vec4( uniform_color.rgb, edge_factor );
}