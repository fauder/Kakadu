#ifndef _INTRINSIC_LIGHTING_GLSL
#define _INTRINSIC_LIGHTING_GLSL

#define POINT_LIGHT_MAX_COUNT 15
#define SPOT_LIGHT_MAX_COUNT 15

struct DirectionalLightData
{
	vec4 ambient, diffuse, specular;
	vec4 direction_view_space;
};

struct PointLightData
{
	vec4 ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_and_attenuation_quadratic;
	vec4 position_view_space;
};

struct SpotLightData
{
	vec4 ambient, diffuse, specular;
	vec4 position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer;
};

layout ( row_major, std140 ) uniform _Intrinsic_Lighting
{
/* Shadow-mapping: */
	mat4x4					_INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM;
	vec4					_INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED;
	ivec2					_INTRINSIC_SHADOW_SAMPLE_COUNT_X_Y;

/* Lighting: */
	uint					_INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE;
	uint					_INTRINSIC_POINT_LIGHT_ACTIVE_COUNT;
	uint					_INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT;

uvec3 padding;

	DirectionalLightData	_INTRINSIC_DIRECTIONAL_LIGHT;
	PointLightData			_INTRINSIC_POINT_LIGHTS[ POINT_LIGHT_MAX_COUNT ];
	SpotLightData			_INTRINSIC_SPOT_LIGHTS[ SPOT_LIGHT_MAX_COUNT ];
};

#endif // _INTRINSIC_LIGHTING_GLSL