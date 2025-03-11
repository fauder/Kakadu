#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_sRGB.glsl"

in VS_Out
{
    vec4 varying_tangent_world_space;
    vec4 varying_bitangent_world_space;
    vec4 varying_normal_world_space;
} fs_in;

out vec4 out_color;

uniform int uniform_show_tangents_bitangents_normals;

void main()
{
    if( uniform_show_tangents_bitangents_normals == 0 ) // Tangents:
    	out_color = sRGBA_To_Linear( fs_in.varying_tangent_world_space * 0.5 + vec4( 0.5, 0.5, 0.5, 1.0f ) );
    else if( uniform_show_tangents_bitangents_normals == 1 ) // Bitangents:
    	out_color = sRGBA_To_Linear( fs_in.varying_bitangent_world_space * 0.5 + vec4( 0.5, 0.5, 0.5, 1.0f ) );
    else if( uniform_show_tangents_bitangents_normals == 2 ) // Normals:
    	out_color = sRGBA_To_Linear( fs_in.varying_normal_world_space * 0.5 + vec4( 0.5, 0.5, 0.5, 1.0f ) );
}