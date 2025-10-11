#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"
#include "_Math.glsl"

uniform vec4 uniform_color; /* _hint_color4 */
uniform float uniform_line_thickness; /* _hint_slider_in_pixels */

#pragma feature OFFSET_DEPTH
#ifdef OFFSET_DEPTH
#define DEPTH_BIAS 1e-5
#endif

in VS_To_FS
{
    noperspective in vec4 edge_info_A;
    noperspective in vec4 edge_info_B;
    flat in uint case_id;
} fs_in;

out vec4 out_color;

vec2 MidpointOfClippedLine( const vec2 p1, const vec2 p2, bool is_infinite_line )
{
    vec2 t_values = ClipLineAgainstNearPlane( p1, p2, is_infinite_line );

    vec2 entry_point = p1 + t_values[ 0 ] * ( p2 - p1 );
    vec2  exit_point = p1 + t_values[ 1 ] * ( p2 - p1 );

    return 0.5 * ( entry_point + exit_point );
}

void main()
{
#ifdef OFFSET_DEPTH
    gl_FragDepth = gl_FragCoord.z - DEPTH_BIAS;
#endif

    out_color.rgb = uniform_color.rgb;

    float min_distance_to_edge;

    if( fs_in.case_id == 0 )
    {
        /* The standard case: Just use the minimum of the distance-to-edge values already calculated in the geometry shader, interpolated and ready to go. */

        min_distance_to_edge = min( min( fs_in.edge_info_A.x, fs_in.edge_info_A.y ), fs_in.edge_info_A.z );

        out_color.a = 1.0;
    }
    else
    {
        /* Tricky case(s): Calculate fragment distances to the edges and use the minimum to determine wireframe status.
         * A, B and C are the vertices 1, 2 and 3 respectively.
         *
         * For case ids 3, 5 & 6, there are 2 points outside the viewport and 1 point inside.
         * This means that the edge of the triangle connected by these invisible points can be omitted completely,
         * As no wireframe will need to be rendered for a completely invisible line. 
         *
         * Thus, code below computes the distance-to-edge values for the always (at least partly) visible 2 edges of the triangle.
         *
         * Refer to the figure 3 in NVIDIA's white paper "Solid Wireframe" (February 2007 WP-03014-001_v01) for further clarification. */

         /* Unpack edge info. calculated in geometry shader: */
        vec2 A_dir = fs_in.edge_info_A.zw;
        vec2 B_dir = fs_in.edge_info_B.zw;
        vec2 AF, BF;
        
        AF = fs_in.edge_info_A.x >= 0 && fs_in.edge_info_A.x <= _INTRINSIC_VIEWPORT_SIZE.x && fs_in.edge_info_A.y >= 0 && fs_in.edge_info_A.y <= _INTRINSIC_VIEWPORT_SIZE.y
                    ? gl_FragCoord.xy - fs_in.edge_info_A.xy
                    : gl_FragCoord.xy - MidpointOfClippedLine( fs_in.edge_info_A.xy, fs_in.edge_info_A.xy + A_dir, true );
        BF = fs_in.edge_info_B.x >= 0 && fs_in.edge_info_B.x <= _INTRINSIC_VIEWPORT_SIZE.x && fs_in.edge_info_B.y >= 0 && fs_in.edge_info_B.y <= _INTRINSIC_VIEWPORT_SIZE.y
                    ? gl_FragCoord.xy - fs_in.edge_info_B.xy
                    : gl_FragCoord.xy - MidpointOfClippedLine( fs_in.edge_info_B.xy, fs_in.edge_info_B.xy + B_dir, true );
        
        /* Use pythagorean theorem to calculate the missing piece that is the minimum distance of the point F (fragment pos.) to the line(s).
         * Hypothenuse(s) are the distances from F to A and B respectively.
         * Known edges are the projections of F onto A_dir and B_dir respectively: */
        float projection_f_onto_A_dir       = dot( A_dir, AF ); // Hypothenuse.
        float projection_f_onto_B_dir       = dot( B_dir, BF ); // Hypothenuse.
        float squared_distance_to_A_A_prime = abs( dot( AF, AF ) - projection_f_onto_A_dir * projection_f_onto_A_dir );
        float squared_distance_to_B_B_prime = abs( dot( BF, BF ) - projection_f_onto_B_dir * projection_f_onto_B_dir );

        float min_squared_distance_to_edge = min( squared_distance_to_A_A_prime, squared_distance_to_B_B_prime );

        /* For cases where 2 points are in-view and 1 points is outside, there is an additional third edge we need to consider:
         * The completely visible edge AB: */
        if( fs_in.case_id == 1 || fs_in.case_id == 2 || fs_in.case_id == 4 )
        {
            vec2 AB_dir = normalize( fs_in.edge_info_B.xy - fs_in.edge_info_A.xy );

            vec2 XF;

            XF = fs_in.edge_info_A.x >= 0 && fs_in.edge_info_A.x <= _INTRINSIC_VIEWPORT_SIZE.x && fs_in.edge_info_A.y >= 0 && fs_in.edge_info_A.y <= _INTRINSIC_VIEWPORT_SIZE.y
                ? AF
                : fs_in.edge_info_B.x >= 0 && fs_in.edge_info_B.x <= _INTRINSIC_VIEWPORT_SIZE.x && fs_in.edge_info_B.y >= 0 && fs_in.edge_info_B.y <= _INTRINSIC_VIEWPORT_SIZE.y
                    ? BF
                    : gl_FragCoord.xy - MidpointOfClippedLine( fs_in.edge_info_A.xy, fs_in.edge_info_B.xy, false );

            float projection_f_onto_AB_dir = dot( AB_dir, XF ); // Hypothenuse.

            float squared_distance_to_AB = abs( dot( XF, XF ) - projection_f_onto_AB_dir * projection_f_onto_AB_dir );
            min_squared_distance_to_edge = min( min_squared_distance_to_edge, squared_distance_to_AB );
        }

        min_distance_to_edge = sqrt( min_squared_distance_to_edge );
    }

    // Cull fragments too far from the edge:
    if( min_distance_to_edge > uniform_line_thickness + 1 )
        discard;

    // Map the computed distance to the [0,2] range on the border of the line.
    min_distance_to_edge = clamp( ( min_distance_to_edge - ( uniform_line_thickness - 1 ) ), 0, 2 );

    // Alpha is computed from the function exp2(-2(x)^2):
    float alpha = exp2( -2 * min_distance_to_edge * min_distance_to_edge );

    out_color.a = alpha;
}