#ifndef _ATTRIBUTES_GLSL
#define _ATTRIBUTES_GLSL

/*
 * Attribute Locations:
 */

/* Main: */
#define POSITION_LOCATION	0
#define NORMAL_LOCATION		1
#define TEXCOORDS_LOCATION	2
#define TANGENT_LOCATION	3

/* Instanced: */
#define INSTANCE_WORLD_TRANSFORM_LOCATION	4
#define INSTANCE_COLOR_LOCATION				INSTANCE_WORLD_TRANSFORM_LOCATION + 4

#define INSTANCED_ATTRIBUTE_START INSTANCE_WORLD_TRANSFORM_LOCATION

/*
 * Attribute Definitions:
 */

/* Main: */
#define POSITION					layout (location = POSITION_LOCATION ) in
#define NORMAL						layout (location = NORMAL_LOCATION ) in
#define TEXCOORDS					layout (location = TEXCOORDS_LOCATION ) in
#define TANGENT						layout (location = TANGENT_LOCATION	) in

/* Instanced: */
#define INSTANCE_WORLD_TRANSFORM	layout (location = INSTANCE_WORLD_TRANSFORM_LOCATION ) in
#define INSTANCE_COLOR				layout (location = INSTANCE_COLOR_LOCATION ) in

#endif // _ATTRIBUTES_GLSL