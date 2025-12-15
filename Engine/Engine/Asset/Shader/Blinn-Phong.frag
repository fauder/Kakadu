#version 460 core
#extension GL_ARB_shading_language_include : require

#include "_Intrinsic_Lighting.glsl"

#pragma feature SKYBOX_ENVIRONMENT_MAPPING
#pragma feature SHADOWS_ENABLED
#pragma feature SOFT_SHADOWS
#pragma feature PARALLAX_MAPPING_ENABLED

in VS_To_FS
{
	vec4 position_view_space;
    vec4 surface_normal_view_space;
    vec2 tex_coords;

    mat3x3 tangent_to_view_space_transformation;

#ifdef PARALLAX_MAPPING_ENABLED
    vec3 viewing_direction_tangent_space;
#endif

#ifdef SHADOWS_ENABLED
    vec4 position_light_directional_clip_space;
#endif
} fs_in;

out vec4 out_color;

layout ( std140 ) uniform BlinnPhongMaterialData
{
/* These 2 are combined into a vec4 on cpu side. */
	vec3 color_diffuse; /* _hint_color3 */
	bool has_texture_diffuse;

	vec3 color_emission; /* _hint_color3 */
	float shininess;
} uniform_blinn_phong_material_data;

uniform sampler2D uniform_tex_diffuse;
uniform sampler2D uniform_tex_specular;
uniform sampler2D uniform_tex_normal;
uniform sampler2D uniform_tex_emission;

#ifdef SKYBOX_ENVIRONMENT_MAPPING
uniform samplerCube uniform_tex_skybox;
uniform sampler2D uniform_tex_reflection;
uniform float uniform_reflectivity; /* _hint_normalized_percentage */
#endif

#ifdef SHADOWS_ENABLED
uniform sampler2D uniform_tex_shadow;
#endif

#ifdef PARALLAX_MAPPING_ENABLED
uniform sampler2D uniform_tex_parallax_height;
uniform float uniform_parallax_height_scale;
uniform uvec2 uniform_parallax_depth_layer_count_min_max;
#endif

#ifdef SHADOWS_ENABLED
/* Returns either 1 = in-shadow or 0 = not in shadow. */
float CalculateShadowAmount( float light_dot_normal )
{
	vec3 ndc       = fs_in.position_light_directional_clip_space.xyz / fs_in.position_light_directional_clip_space.w;
	vec3 ndc_unorm = ndc * 0.5f + 0.5f;

	/* Since position_light_directional_clip_space values are not actually clipped, there may be values outside the frustum of the light.
	 * This means that the ndc_unorm above is not actually unorm: Values outside the frustum are naturally mapped outside the [0,1] range. 
	 * Thus, values bigger than 1.0 are clamped to 0 (= no shadow). */

	if( ndc_unorm.z > 1.0f )
		return 0.0f;

	float current_depth = ndc_unorm.z;

#ifdef SOFT_SHADOWS
	float shadow = 0.0f;
	vec2 texel_size = 1.0f / textureSize( uniform_tex_shadow, 0 );

	int x_limit = _INTRINSIC_SHADOW_SAMPLE_COUNT_X_Y.x / 2;
	int y_limit = _INTRINSIC_SHADOW_SAMPLE_COUNT_X_Y.y / 2;
	float sample_count = ( x_limit * 2 + 1 ) * ( y_limit * 2 + 1 );

	for( int x = -x_limit; x < x_limit; x++ )
	{
		for( int y = -y_limit; y < y_limit; y++ )
		{
			float shadow_map_sample_z = texture( uniform_tex_shadow, ndc_unorm.xy + vec2( x, y ) * texel_size ).r;

			float bias = max( _INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED.y * ( 1.0f - light_dot_normal ), _INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED.x );  
			shadow += ( current_depth - bias ) > shadow_map_sample_z ? 1.0f : 0.0f;
		}
	}

	return shadow / sample_count;
#else
	float shadow_map_sample_z = texture( uniform_tex_shadow, ndc_unorm.xy ).r;

	float bias = max( _INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED.y * ( 1.0f - light_dot_normal ), _INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED.x );  
	return ( current_depth - bias ) > shadow_map_sample_z ? 1.0f : 0.0f;
#endif
}
#endif

vec3 CalculateColorFromDirectionalLight( vec4 normal_view_space, vec4 viewing_direction_view_space,
										 vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_DIRECTIONAL_LIGHT.ambient.rgb;

/* Diffuse term: */
	vec4 to_light_view_space = normalize( -_INTRINSIC_DIRECTIONAL_LIGHT.direction_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0f );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_DIRECTIONAL_LIGHT.diffuse.rgb * diffuse_contribution;

/* Specular term: */
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0f ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_DIRECTIONAL_LIGHT.specular.rgb * specular_contribution;

#ifdef SHADOWS_ENABLED
	float shadow = CalculateShadowAmount( dot( _INTRINSIC_DIRECTIONAL_LIGHT.direction_view_space, normal_view_space ) );
	return ambient + ( 1.0f - shadow ) * ( diffuse + specular );
#else
	return ambient + diffuse + specular;
#endif
}

float CalculateAttenuation( const int point_light_index, float distance )
{
	return 1.0f / ( _INTRINSIC_POINT_LIGHTS[ point_light_index ].ambient_and_attenuation_constant.w +
					_INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.w	  * distance + 
					_INTRINSIC_POINT_LIGHTS[ point_light_index ].specular_and_attenuation_quadratic.w * distance * distance );
}

vec3 CalculateColorFromPointLight( const int point_light_index, 
								   vec4 normal_view_space, vec4 viewing_direction_view_space,
								   vec3 diffuse_sample, vec3 specular_sample )
{
/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].ambient_and_attenuation_constant.rgb;

/* Diffuse term: */
	// Uniform point light data passed is passed as a vector3 (padded to vector4); w component is not sent by the CPU side, so it can be any value. Set to 1.0 here to make sure.
	vec4 to_light_view_space = normalize( vec4( _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz, 1.0 ) - fs_in.position_view_space );

	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0f );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].diffuse_and_attenuation_linear.rgb * diffuse_contribution;

/* Specular term: */
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0f ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_POINT_LIGHTS[ point_light_index ].specular_and_attenuation_quadratic.rgb * specular_contribution;

/* Attenuation: */
	float distance_view_space = distance( fs_in.position_view_space.xyz, _INTRINSIC_POINT_LIGHTS[ point_light_index ].position_view_space.xyz );
	float attenuation         = CalculateAttenuation( point_light_index, distance_view_space );

	ambient  *= attenuation;
	diffuse  *= attenuation;
	specular *= attenuation;

	return vec3( ambient + diffuse + specular );
}

vec3 CalculateColorFromSpotLight( const int spot_light_index, 
								  vec4 normal_view_space, vec4 viewing_direction_view_space,
							      vec3 diffuse_sample, vec3 specular_sample )
{
	vec3 light_position_view_space  = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].position_view_space_and_cos_cutoff_angle_inner.xyz;
	vec3 direction_view_space       = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].direction_view_space_and_cos_cutoff_angle_outer.xyz;

	float cos_cutoff_angle_inner = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].position_view_space_and_cos_cutoff_angle_inner.w;
	float cos_cutoff_angle_outer = _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].direction_view_space_and_cos_cutoff_angle_outer.w;

	vec4 to_light_view_space   = normalize( vec4( light_position_view_space, 1.0 ) - fs_in.position_view_space );
	vec4 from_light_view_space = -to_light_view_space;

	float cut_off_intensity	= clamp( ( dot( from_light_view_space.xyz, direction_view_space ) - cos_cutoff_angle_outer ) /
									 ( cos_cutoff_angle_inner - cos_cutoff_angle_outer ),
									 0, 1 );

/* Ambient term: */
	vec3 ambient = diffuse_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].ambient.rgb;

/* Diffuse term: */
	float diffuse_contribution = max( dot( to_light_view_space, normal_view_space ), 0.0f );
	vec3 diffuse               = diffuse_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].diffuse.rgb * diffuse_contribution;

/* Specular term: */
	vec4 halfway_angle_view_space = normalize( to_light_view_space + viewing_direction_view_space );

	float specular_contribution = pow( max( dot( halfway_angle_view_space, normal_view_space ), 0.0f ), uniform_blinn_phong_material_data.shininess );
	vec3 specular               = specular_sample * _INTRINSIC_SPOT_LIGHTS[ spot_light_index ].specular.rgb * specular_contribution;

	return ambient + cut_off_intensity * ( diffuse + specular );
}

#ifdef PARALLAX_MAPPING_ENABLED
vec2 ParallaxMappedTextureCoordinates( vec2 original_texture_coordinates, vec3 viewing_direction_tangent_space )
{
	float height_sample = texture( uniform_tex_parallax_height, original_texture_coordinates ).r;

	// Use the lerp. range max-min, as we want to minimize the sample count when the viewing direction aligns with the normal.
	const float layer_count = float( mix( uniform_parallax_depth_layer_count_min_max.y, uniform_parallax_depth_layer_count_min_max.x,
										  max( dot( vec3( 0.0f, 0.0f, +1.0f ), viewing_direction_tangent_space ), 0.0f ) ) );

	const float layer_depth = 1.0f / layer_count;
	float current_level_depth_value = 0.0f;

	const vec2 delta_uv = viewing_direction_tangent_space.xy * uniform_parallax_height_scale / layer_count;

	vec2 current_uv = original_texture_coordinates;

	/* It's important to initialize previous_uv to current_uv, as the outermost (i.e., least "dented") fragments may skip the while loop due to having a bigger depth value than the initial heightmap sample. */
	vec2 previous_uv = current_uv;
	float previous_height_sample;
	float previous_level_depth_value;

	while( current_level_depth_value < height_sample )
	{
		previous_uv = current_uv;
		current_uv -= delta_uv;

		previous_height_sample = height_sample;
		height_sample          = texture( uniform_tex_parallax_height, current_uv ).r;

		previous_level_depth_value = current_level_depth_value;
		current_level_depth_value += layer_depth;
	}

	/* From similar triangles: */
	float current_height_difference  = current_level_depth_value - height_sample; // Subtract order is adjusted so that the difference is positive.
	float previous_height_difference = previous_height_sample - previous_level_depth_value;

	float interpolation_weight = clamp( previous_height_difference / ( previous_height_difference + current_height_difference ), 0.0f, 1.0f ); // Clamp to avoid INFs when the divisor is zero.

	current_uv = mix( previous_uv, current_uv, interpolation_weight );

	return current_uv;
}
#endif

void main()
{
	// No need to subtract from the camera position since the camera is positioned at the origin in view space.
	vec4 viewing_direction_view_space = normalize( -fs_in.position_view_space ); 

#ifdef PARALLAX_MAPPING_ENABLED
	vec2 uvs = ParallaxMappedTextureCoordinates( fs_in.tex_coords, normalize( fs_in.viewing_direction_tangent_space ) );
	if( uvs.x > 1.0 || uvs.y > 1.0 || uvs.x < 0.0 || uvs.y < 0.0 )
		discard;
#else
	vec2 uvs = fs_in.tex_coords;
#endif

	/* Normally, the convention used in this engine is to post-multiply vectors with matrices BUT
	 * tangent_to_view_space_transformation is constructed in the vertex shader part of this shader program => it is column-major.
	 * That's why in the below line, the sampled tangent-space normal vector is pre-multiplied by the column-major tangent_to_view_space_transformation matrix. */

    vec4 normal_sample_view_space = vec4( normalize( fs_in.tangent_to_view_space_transformation * ( texture( uniform_tex_normal, uvs ).xyz * 2.0f - 1.0f ) ),
										  0.0f );

//	vec4 surface_normal_view_space = normalize( fs_in.surface_normal_view_space );

	vec3 diffuse_sample  = uniform_blinn_phong_material_data.has_texture_diffuse
							   ? vec3( texture( uniform_tex_diffuse, uvs ) )
							   : uniform_blinn_phong_material_data.color_diffuse;
	vec3 specular_sample = vec3( texture( uniform_tex_specular, uvs ) );

	vec3 from_directional_light = _INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE * 
									CalculateColorFromDirectionalLight( normal_sample_view_space, viewing_direction_view_space,
																		diffuse_sample, specular_sample );

	vec3 emission = texture( uniform_tex_emission, uvs ).rgb;

	vec3 from_point_light = vec3( 0 );																		
	for( int i = 0; i < _INTRINSIC_POINT_LIGHT_ACTIVE_COUNT; i++ )
		from_point_light += CalculateColorFromPointLight( i,
														  normal_sample_view_space, viewing_direction_view_space,
														  diffuse_sample, specular_sample );

	vec3 from_spot_light = vec3( 0 );																		
	for( int i = 0; i < _INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT; i++ )
		from_spot_light += CalculateColorFromSpotLight( i,
														normal_sample_view_space, viewing_direction_view_space,
														diffuse_sample, specular_sample );

	out_color = vec4( from_directional_light + from_point_light + from_spot_light +
					  emission * uniform_blinn_phong_material_data.color_emission,
					  1.0 );

#ifdef SKYBOX_ENVIRONMENT_MAPPING
	// TODO: Calculate the reflected vector in the vertex shader AND make sure it is in world space.
	vec3 reflected         = reflect( fs_in.position_view_space, normal_sample_view_space ).xyz;
	vec4 reflection_sample = vec4( texture( uniform_tex_skybox, reflected.xyz ).rgb, 1.0 );

	vec4 reflection_map_sample = vec4( texture( uniform_tex_reflection, uvs ).rgb, 1.0 );

	out_color = mix( out_color, ( 1.0 - reflection_map_sample.r ) * reflection_sample, uniform_reflectivity );
#endif
}