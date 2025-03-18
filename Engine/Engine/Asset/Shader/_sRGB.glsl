float sRGB_To_Linear_Single( float sRGB_color_channel )
{
    float comparison_result = float( sRGB_color_channel <= 0.04045f );
    return ( comparison_result * sRGB_color_channel / 12.92f ) + ( ( 1.0f - comparison_result ) * pow( ( sRGB_color_channel + 0.055f ) / 1.055f, 2.4f ) );
}

vec3 sRGB_To_Linear( vec3 sRGB_color )
{
    return vec3( sRGB_To_Linear_Single( sRGB_color.r ),
                 sRGB_To_Linear_Single( sRGB_color.g ),
                 sRGB_To_Linear_Single( sRGB_color.b ) );
}

vec4 sRGBA_To_Linear( vec4 sRGBA_color )
{
    return vec4( sRGB_To_Linear( sRGBA_color.rgb ), sRGBA_color.a );
}

float Linear_To_sRGB_Single( float linear_color_channel )
{
    float comparison_result = float( linear_color_channel <= 0.0031308f );
    return ( comparison_result * linear_color_channel * 12.92f ) + ( ( 1.0f - comparison_result ) * ( 1.055f * pow( linear_color_channel, 1.0f / 2.4f ) - 0.055f ) );
}

vec3 Linear_To_sRGB( vec3 linear_color )
{
    return vec3( Linear_To_sRGB_Single( linear_color.r ),
                 Linear_To_sRGB_Single( linear_color.g ),
                 Linear_To_sRGB_Single( linear_color.b ) );
}

vec4 Linear_To_sRGBA( vec4 linear_color )
{
    return vec4( Linear_To_sRGB( linear_color.rgb ), linear_color.a );
}