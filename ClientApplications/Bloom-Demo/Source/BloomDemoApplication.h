// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/ModelInstance.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/RHI/Texture.h"

#include "Engine/DefineMathTypes.h"

// std Includes.
#include <numeric> // std::accumulate().

class BloomDemoApplication : public Kakadu::Application
{
	DEFINE_MATH_TYPES()

	struct ModelInfo
	{
		Kakadu::ModelInstance model_instance;
		Kakadu::RHI::Shader* shader;
		Kakadu::RHI::Shader* shader_shadow_receiving;
		std::string file_path;
		bool is_receiving_shadows;
		bool is_casting_shadows;
	};

public:
	BloomDemoApplication( const Kakadu::BitFlags< Kakadu::CreationFlags > );
	virtual ~BloomDemoApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void RenderFrame() override;

	virtual void RenderToolsUI() override;

	virtual void OnMouseButtonEvent( const Kakadu::Platform::MouseButton button, const Kakadu::Platform::MouseButtonAction button_action, const Kakadu::Platform::KeyMods key_mods ) override;
	virtual void OnMouseScrollEvent( const float x_offset, const float y_offset ) override;
	virtual void OnKeyboardEvent( const Kakadu::Platform::KeyCode key_code, const Kakadu::Platform::KeyAction key_action, const Kakadu::Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const i32 width_new_pixels, const i32 height_new_pixels ) override;

private:
	void ResetLightingData();
	void ResetMaterialData();
	void ResetInstanceData();

	bool ReloadModel( ModelInfo& model_info_to_be_loaded, const std::string& file_path, const char* name );
	void UnloadModel( ModelInfo& model_info_to_be_unloaded, const char* name );

	void ReplaceMeteoriteAndCubeRenderables( bool use_meteorites );

private:
	/* Renderer: */
	Kakadu::Renderable light_sources_renderable;
	Kakadu::Renderable stars_renderable;

	const static constexpr i32 CUBE_COUNT = 200'000;
	const static constexpr i32 CUBE_REFLECTED_COUNT = 10;
	Kakadu::Renderable cube_renderable;
	Kakadu::Renderable cube_renderable_outline;

	Kakadu::Renderable sphere_renderable;

	Kakadu::Renderable cube_parallax_renderable;

	Kakadu::Renderable cube_reflected_renderable;

	Kakadu::Renderable* meteorite_renderable;

	Kakadu::Renderable ground_renderable;
	Kakadu::Renderable wall_front_renderable;
	Kakadu::Renderable wall_left_renderable;
	Kakadu::Renderable wall_right_renderable;
	Kakadu::Renderable wall_back_renderable;

	const static constexpr i32 WINDOW_COUNT = 5;
	std::array< Kakadu::Renderable, WINDOW_COUNT > window_renderable_array;

	/* Textures: */
	Kakadu::RHI::Texture* container_texture_diffuse_map;
	Kakadu::RHI::Texture* container_texture_specular_map;
	Kakadu::RHI::Texture* container_texture_normal_map;

	Kakadu::RHI::Texture* brickwall_diffuse_map;
	Kakadu::RHI::Texture* brickwall_normal_map;
	Kakadu::RHI::Texture* brickwall_displacement_map;

	Kakadu::RHI::Texture* checker_pattern_texture;
	Kakadu::RHI::Texture* transparent_window_texture;

	/* Vertex Info.: */
	Kakadu::Mesh cube_mesh, quad_mesh, quad_mesh_uvs_only, quad_mesh_fullscreen, quad_mesh_mirror;
	Kakadu::Mesh cube_mesh_instanced;
	Kakadu::Mesh cube_reflected_mesh_instanced;
	Kakadu::Mesh sphere_mesh_instanced_with_color; // For light sources.
	Kakadu::Mesh sphere_mesh, sphere_mesh_lower_detail;
	Kakadu::Mesh triangle_mesh_instanced_with_color; // For stars.
	Kakadu::Mesh triangle_mesh_positions_only;

	/* Shaders: */
	Kakadu::RHI::Shader* shader_blinn_phong;
	Kakadu::RHI::Shader* shader_blinn_phong_shadowed;
	Kakadu::RHI::Shader* shader_blinn_phong_shadowed_parallax;
	Kakadu::RHI::Shader* shader_blinn_phong_instanced;
	Kakadu::RHI::Shader* shader_blinn_phong_shadowed_instanced;
	Kakadu::RHI::Shader* shader_blinn_phong_shadowed_parallax_instanced;
	Kakadu::RHI::Shader* shader_blinn_phong_skybox_reflection;
	Kakadu::RHI::Shader* shader_blinn_phong_skybox_reflection_instanced;
	Kakadu::RHI::Shader* shader_blinn_phong_skybox_reflection_shadowed_instanced;
	Kakadu::RHI::Shader* shader_basic_color;
	Kakadu::RHI::Shader* shader_basic_color_instanced;
	Kakadu::RHI::Shader* shader_basic_textured;
	Kakadu::RHI::Shader* shader_basic_textured_transparent_discard;
	Kakadu::RHI::Shader* shader_outline;

/* Models: */
	ModelInfo test_model_info;
	ModelInfo meteorite_model_info;

/* Materials: */
	Kakadu::Material light_source_material;

	Kakadu::Material cube_material;
	Kakadu::Material cube_reflected_material;

	Kakadu::Material ground_material;
	Kakadu::Material wall_material;

	Kakadu::Material window_material;

	Kakadu::Material outline_material;

	Kakadu::Material sphere_material;

	Kakadu::Material star_material;

/* Instancing Data: */

	struct LightInstanceData
	{
		Matrix4x4 transform;
		Kakadu::Color4 color;
	};

	std::vector< Matrix4x4 > cube_instance_data_array;
	std::vector< Matrix4x4 > cube_reflected_instance_data_array;
	std::vector< LightInstanceData > light_source_instance_data_array;
	std::vector< LightInstanceData > star_instance_data_array;

/* Scene: */
	/* Lights: */
	Kakadu::Transform light_directional_transform;
	std::vector< Kakadu::Transform > light_point_transform_array;
	Kakadu::Transform light_spot_transform;

	/* GameObjects: */
	std::vector< Kakadu::Transform > cube_transform_array;
	std::vector< Kakadu::Transform > cube_reflected_transform_array;

	std::vector< Kakadu::Transform > star_transform_array;

	Kakadu::Transform cube_parallax_transform;
	Kakadu::Transform ground_transform;
	Kakadu::Transform wall_front_transform;
	Kakadu::Transform wall_left_transform;
	Kakadu::Transform wall_right_transform;
	Kakadu::Transform wall_back_transform;

	Kakadu::Transform sphere_transform;

	std::array< Kakadu::Transform, WINDOW_COUNT > window_transform_array;

/* Lighting: */
	const static constexpr i32 STAR_COUNT = 1000;
	const static constexpr i32 LIGHT_POINT_COUNT = 15;

	Kakadu::MaterialData::BlinnPhongMaterialData cube_surface_data;
	Kakadu::MaterialData::BlinnPhongMaterialData ground_quad_surface_data;
	Kakadu::MaterialData::BlinnPhongMaterialData wall_surface_data;
	std::vector< Kakadu::MaterialData::BlinnPhongMaterialData > test_model_node_surface_data_array;

	Kakadu::DirectionalLight light_directional;
	std::vector< Kakadu::PointLight > light_point_array;
	Kakadu::SpotLight light_spot;

	bool light_is_enabled;
	bool light_point_array_disable;
	bool light_point_array_is_animated;
	float light_point_orbit_radius;

	bool light_spot_array_disable;

/* Frame statistics: */
	Radians current_time_as_angle;

/* Other: */
	const static constexpr std::array< Vector3, 10 > CUBE_REFLECTED_POSITIONS =
	{ {
		{  0.0f,  0.0f,  0.0f	},
		{  2.0f,  5.0f, +15.0f	},
		{ -1.5f, -2.2f, +2.5f	},
		{ -3.8f, -2.0f, +12.3f	},
		{  2.4f, -0.4f, +3.5f	},
		{ -1.7f,  3.0f, +7.5f	},
		{  1.3f, -2.0f, +2.5f	},
		{  1.5f,  2.0f, +2.5f	},
		{  1.5f,  0.2f, +1.5f	},
		{ -1.3f,  1.0f, +1.5f	}
	} };

	const static constexpr Vector3 CUBES_ORIGIN = std::accumulate( CUBE_REFLECTED_POSITIONS.cbegin(), CUBE_REFLECTED_POSITIONS.cend(), Vector3::Zero() ) / CUBE_COUNT;
	const static constexpr Vector3 CAMERA_ROTATION_ORIGIN = CUBES_ORIGIN;
};