#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Attributes.glsl"

POSITION vec3 position;

void main()
{
    gl_Position = vec4( position, 1.0 );
}