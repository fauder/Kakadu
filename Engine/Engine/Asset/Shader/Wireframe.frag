#version 460 core

in vec3 varying_barycentric_coordinates;

out vec4 out_color;

uniform vec4 uniform_color; /* _hint_color4 */
uniform float uniform_threshold; /* _hint_normalized_percentage */

void main()
{
    float min_bary    = min( min( varying_barycentric_coordinates.x, varying_barycentric_coordinates.y ), varying_barycentric_coordinates.z );
    float edge_factor = 1.0f - smoothstep( 0.0, uniform_threshold, min_bary );

    out_color = vec4( uniform_color.rgb, edge_factor );
}