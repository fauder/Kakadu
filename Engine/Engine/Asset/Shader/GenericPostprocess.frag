#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Other.glsl"

out vec4 out_color;

uniform sampler2D uniform_tex;

#define KERNEL_WIDTH 3
#define KERNEL_HEIGHT 3

//layout ( std140 ) uniform KernelData
//{
//    float values[ KERNEL_WIDTH * KERNEL_HEIGHT ];
//} uniform_kernel_data;

#pragma array( 3, 3 )
uniform float uniform_kernel[ KERNEL_WIDTH * KERNEL_HEIGHT ];

void main()
{
    vec4 samples[ KERNEL_WIDTH * KERNEL_HEIGHT ];

    int kernel_to_texel_index_offset_horizontal  = KERNEL_WIDTH  / 2;
    int kernel_to_texel_index_offset_vertical    = KERNEL_HEIGHT / 2;

    for( int row_index = 0; row_index < KERNEL_HEIGHT; row_index++ )
    {
        int kernel_index_vertical = row_index - kernel_to_texel_index_offset_vertical;
        for( int column_index = 0; column_index < KERNEL_WIDTH; column_index++ )
        {
            int kernel_index_horizontal = column_index - kernel_to_texel_index_offset_horizontal;
            samples[ row_index * KERNEL_WIDTH + column_index ] = texture( uniform_tex, gl_FragCoord.xy + vec2( float( kernel_index_horizontal ) / _INTRINSIC_VIEWPORT_SIZE.x, 
                                                                                                               float( kernel_index_vertical   ) / _INTRINSIC_VIEWPORT_SIZE.y ) );
        }
    }
	
    out_color = vec4( vec3( 0.0 ), 1.0 );
    for( int i = 0; i < KERNEL_WIDTH * KERNEL_HEIGHT; i++ )
        out_color += samples[ i ] * /*uniform_kernel_data.values[ i ]*/ uniform_kernel[ i ];
}
