#ifndef _GLOBAL_GLSL
#define _GLOBAL_GLSL

layout ( std140 ) uniform _Global
{
	ivec2 _GLOBAL_CURSOR_POS_SCREEN_SPACE;
	int _GLOBAL_OFFSCREEN_VERTEX_HANDLING_METHOD;
	// 4 bytes of padding.
};

#endif // _GLOBAL_GLSL