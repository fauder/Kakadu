#version 460 core

#include "_Intrinsic_Other.glsl"
#include "_Math.glsl"

layout ( triangles ) in;
layout ( triangle_strip, max_vertices = 3 ) out;

out VS_To_FS
{
    noperspective vec4 edge_info_A;  /* Components: XY = Screen-space point  A. ZW = Screen-space direction  A-dir, pointing toward an off-frustum vertex. */
    noperspective vec4 edge_info_B;  /* Components: XY = Screen-space point  B. ZW = Screen-space direction  B-dir, pointing toward an off-frustum vertex. */
    noperspective vec4 edge_info_AB; /* Components: XY = Screen-space point AB. ZW = Screen-space direction AB-dir. */
    flat uint case_id;
} vs_out;

/*                              P0.z > 0:  T  T  T  T  F  F  F  F
                                P1.z > 0:  T  T  F  F  T  T  F  F 
                                P2.z > 0:  T  F  T  F  T  F  T  F
                                           ----------------------
                       Case ID: [EASY CASE 0] 1  2  3  4  5  6 [7 TRIANGLE NOT VISIBLE => CULLED] */
const int TABLE_INDICES_FOR_A[ 8 ]     = { 0, 0, 0, 0, 1, 1, 2, 0 };
const int TABLE_INDICES_FOR_B[ 8 ]     = { 0, 1, 2, 0, 2, 1, 2, 0 };
const int TABLE_INDICES_FOR_A_DIR[ 8 ] = { 0, 2, 1, 1, 0, 0, 0, 0 };
const int TABLE_INDICES_FOR_B_DIR[ 8 ] = { 0, 2, 1, 2, 0, 2, 1, 0 };

vec2 TransformToScreenSpace( vec4 clip_space_point )
{
    return ( ( clip_space_point.xy / clip_space_point.w ) // Divide by w to convert to NDC.
             * 0.5 + 0.5 )                                // Scale & Translate the origin
           * _INTRINSIC_VIEWPORT_SIZE.xy;                 // Scale by the screen resolution.
}

vec2 MidpointOfClippedLine( const vec2 p1, const vec2 p2, bool is_infinite_line )
{
    vec2 t_values = ClipLineAgainstViewport( p1, p2, is_infinite_line );

    vec2 entry_point = p1 + t_values[ 0 ] * ( p2 - p1 );
    vec2  exit_point = p1 + t_values[ 1 ] * ( p2 - p1 );

    return 0.5 * ( entry_point + exit_point );
}

void main()
{
    vec2 screen_space_points[ 3 ] =
    {
        TransformToScreenSpace( gl_in[ 0 ].gl_Position ),
        TransformToScreenSpace( gl_in[ 1 ].gl_Position ),
        TransformToScreenSpace( gl_in[ 2 ].gl_Position )
    };

    /* Case determination: */
    vs_out.case_id = uint( gl_in[ 0 ].gl_Position.z < 0 ) * 4 +
                     uint( gl_in[ 1 ].gl_Position.z < 0 ) * 2 +
                     uint( gl_in[ 2 ].gl_Position.z < 0 );

    if( vs_out.case_id == 7 )
        return; // Cull invisible triangles.

    if( vs_out.case_id != 0 )
    {
        /* Tricky case(s): Calculate edge info.: */
        vs_out.edge_info_A.xy = screen_space_points[ TABLE_INDICES_FOR_A[ vs_out.case_id ] ];
        vs_out.edge_info_B.xy = screen_space_points[ TABLE_INDICES_FOR_B[ vs_out.case_id ] ];

        vs_out.edge_info_A.zw = normalize( screen_space_points[ TABLE_INDICES_FOR_A_DIR[ vs_out.case_id ] ] - vs_out.edge_info_A.xy  );
        vs_out.edge_info_B.zw = normalize( screen_space_points[ TABLE_INDICES_FOR_B_DIR[ vs_out.case_id ] ] - vs_out.edge_info_B.xy  );

        /* Before clipping A and B, calculate clipping for the AB edge, as it relies on unclipped A & B data. */
        if( vs_out.case_id == 1 || vs_out.case_id == 2 || vs_out.case_id == 4 )
        {
            vs_out.edge_info_AB.zw = normalize( vs_out.edge_info_B.xy - vs_out.edge_info_A.xy );
            
            vs_out.edge_info_AB.xy = vs_out.edge_info_A.x >= 0 && vs_out.edge_info_A.x <= _INTRINSIC_VIEWPORT_SIZE.x && vs_out.edge_info_A.y >= 0 && vs_out.edge_info_A.y <= _INTRINSIC_VIEWPORT_SIZE.y
                ? vs_out.edge_info_A.xy
                : vs_out.edge_info_B.x >= 0 && vs_out.edge_info_B.x <= _INTRINSIC_VIEWPORT_SIZE.x && vs_out.edge_info_B.y >= 0 && vs_out.edge_info_B.y <= _INTRINSIC_VIEWPORT_SIZE.y
                    ? vs_out.edge_info_B.xy
                    : MidpointOfClippedLine( vs_out.edge_info_A.xy, vs_out.edge_info_B.xy, false );
        }

        /* Clip A and B edges: */
        if( vs_out.edge_info_A.x < 0 || vs_out.edge_info_A.x > _INTRINSIC_VIEWPORT_SIZE.x || vs_out.edge_info_A.y < 0 || vs_out.edge_info_A.y > _INTRINSIC_VIEWPORT_SIZE.y )
            vs_out.edge_info_A.xy = MidpointOfClippedLine( vs_out.edge_info_A.xy, vs_out.edge_info_A.xy + vs_out.edge_info_A.zw, true );
        if( vs_out.edge_info_B.x < 0 || vs_out.edge_info_B.x > _INTRINSIC_VIEWPORT_SIZE.x || vs_out.edge_info_B.y < 0 || vs_out.edge_info_B.y > _INTRINSIC_VIEWPORT_SIZE.y )
            vs_out.edge_info_B.xy = MidpointOfClippedLine( vs_out.edge_info_B.xy, vs_out.edge_info_B.xy + vs_out.edge_info_B.zw, true );

        /* Emit vertices normally: */

        gl_Position = gl_in[ 0 ].gl_Position;
        EmitVertex();

        gl_Position = gl_in[ 1 ].gl_Position;
        EmitVertex();

        gl_Position = gl_in[ 2 ].gl_Position;
        EmitVertex();

        EndPrimitive();
    }
    else
    {
        /* The standard case: Do most of the work here in the geometry shader.
         * A, B and C are the vertices 1, 2 and 3 respectively. */

        vec2 AB = screen_space_points[ 1 ] - screen_space_points[ 0 ]; // Vector from A to B.
        vec2 BC = screen_space_points[ 2 ] - screen_space_points[ 1 ]; // Vector from B to C.
        vec2 CA = screen_space_points[ 0 ] - screen_space_points[ 2 ]; // Vector from C to A.

        vec2 AB_dir = normalize( AB );
        vec2 BC_dir = normalize( BC );
        vec2 CA_dir = normalize( CA );

        /* Use pythagorean theorem to calculate the missing pieces that are the heights of the triangles:
         * Hypothenuse(s) are the edge lengths.
         * Known edges are the projections of edges onto other edges. */
        float projection_AC_onto_AB = abs( dot( CA, AB_dir ) );
        float projection_AB_onto_AC = abs( dot( AB, CA_dir ) );
        float projection_AB_onto_BC = abs( dot( AB, BC_dir ) );

        float AB_height = sqrt( abs( dot( CA, CA ) - projection_AC_onto_AB * projection_AC_onto_AB ) );
        float BC_height = sqrt( abs( dot( AB, AB ) - projection_AB_onto_BC * projection_AB_onto_BC ) );
        float CA_height = sqrt( abs( dot( AB, AB ) - projection_AB_onto_AC * projection_AB_onto_AC ) );

        /* Emit vertices and assign height values per vertex: */

        gl_Position = gl_in[ 0 ].gl_Position;
        vs_out.edge_info_A.xyz = vec3( 0.0f, BC_height, 0.0f );
        EmitVertex();

        gl_Position = gl_in[ 1 ].gl_Position;
        vs_out.edge_info_A.xyz = vec3( 0.0f, 0.0f, CA_height );
        EmitVertex();

        gl_Position = gl_in[ 2 ].gl_Position;
        vs_out.edge_info_A.xyz = vec3( AB_height, 0.0f, 0.0f );
        EmitVertex();

        EndPrimitive();
    }
}
