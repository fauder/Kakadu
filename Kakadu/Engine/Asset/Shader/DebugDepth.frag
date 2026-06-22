#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Depth.glsl"

out vec4 out_color;

void main()
{
	out_color = vec4( vec3( LinearizeDepth( gl_FragCoord.z ) / _INTRINSIC_PROJECTION_FAR ), 1.0 );
}
