#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"

POSITION  vec3 position;
TEXCOORDS vec2 tex_coords;

out vec2 varying_tex_coords;

void main()
{
    gl_Position = vec4( position, 1.0 );
    varying_tex_coords = tex_coords;
}