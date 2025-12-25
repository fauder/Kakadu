// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/ModelInstance.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/DefineMathTypes.h"

// std Includes.
#include <numeric> // std::accumulate().

class SandboxApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

	struct ModelInfo
	{
		Engine::ModelInstance model_instance;
		Engine::Shader* shader;
		Engine::Shader* shader_shadow_receiving;
		std::string file_path;
		bool is_receiving_shadows;
		bool is_casting_shadows;
	};

public:
	SandboxApplication( const Engine::BitFlags< Engine::CreationFlags > );
	virtual ~SandboxApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Update() override;

	virtual void RenderFrame() override;

	virtual void RenderImGui() override;
	virtual void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods ) override;
	virtual void OnMouseScrollEvent( const float x_offset, const float y_offset ) override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;
	
private:
	void ResetLightingData();
	void ResetMaterialData();
	void ResetInstanceData();

	bool ReloadModel( ModelInfo& model_info_to_be_loaded, const std::string& file_path, const char* name );
	void UnloadModel( ModelInfo& model_info_to_be_unloaded, const char* name );

	void ReplaceMeteoriteAndCubeRenderables( bool use_meteorites );

private:
/* Renderer: */
	Engine::Renderable light_sources_renderable;

	const static constexpr int CUBE_COUNT           = 200'000;
	const static constexpr int CUBE_REFLECTED_COUNT = 10;
	Engine::Renderable cube_renderable;
	Engine::Renderable cube_renderable_outline;

	Engine::Renderable sphere_renderable;

	Engine::Renderable cube_parallax_renderable;

	Engine::Renderable cube_reflected_renderable;

	Engine::Renderable* meteorite_renderable;

	Engine::Renderable skybox_renderable;

	Engine::Renderable ground_renderable;
	Engine::Renderable wall_front_renderable;
	Engine::Renderable wall_left_renderable;
	Engine::Renderable wall_right_renderable;
	Engine::Renderable wall_back_renderable;

	const static constexpr int WINDOW_COUNT = 5;
	std::array< Engine::Renderable, WINDOW_COUNT > window_renderable_array;

	Engine::Renderable mirror_quad_renderable;

	static constexpr Engine::RenderPass::ID RENDER_PASS_ID_LIGHTING_REAR_VIEW = Engine::RenderPass::ID( ( unsigned int )Engine::Renderer::RENDER_PASS_ID_LIGHTING - 1 );

/* Textures: */
	Engine::Texture* skybox_texture;

	Engine::Texture* container_texture_diffuse_map;
	Engine::Texture* container_texture_specular_map;
	Engine::Texture* container_texture_normal_map;

	Engine::Texture* brickwall_diffuse_map;
	Engine::Texture* brickwall_normal_map;
	Engine::Texture* brickwall_displacement_map;

	Engine::Texture* checker_pattern_texture;
	Engine::Texture* transparent_window_texture;

/* Vertex Info.: */
	Engine::Mesh cube_mesh, cube_mesh_fullscreen, quad_mesh, quad_mesh_uvs_only, quad_mesh_fullscreen, quad_mesh_mirror;
	Engine::Mesh cube_mesh_instanced;
	Engine::Mesh cube_reflected_mesh_instanced;
	Engine::Mesh sphere_mesh_instanced_with_color; // For light sources.
	Engine::Mesh sphere_mesh, sphere_mesh_lower_detail;

/* Shaders: */
	Engine::Shader* shader_skybox;
	Engine::Shader* shader_blinn_phong;
	Engine::Shader* shader_blinn_phong_shadowed;
	Engine::Shader* shader_blinn_phong_shadowed_parallax;
	Engine::Shader* shader_blinn_phong_instanced;
	Engine::Shader* shader_blinn_phong_shadowed_instanced;
	Engine::Shader* shader_blinn_phong_shadowed_parallax_instanced;
	Engine::Shader* shader_blinn_phong_skybox_reflection;
	Engine::Shader* shader_blinn_phong_skybox_reflection_instanced;
	Engine::Shader* shader_blinn_phong_skybox_reflection_shadowed_instanced;
	Engine::Shader* shader_basic_color;
	Engine::Shader* shader_basic_color_instanced;
	Engine::Shader* shader_basic_textured;
	Engine::Shader* shader_basic_textured_transparent_discard;
	Engine::Shader* shader_outline;

	Engine::Shader* shader_texture_blit;

/* Models: */
	ModelInfo test_model_info;
	ModelInfo meteorite_model_info;

/* Materials: */
	Engine::Material skybox_material;

	Engine::Material light_source_material;

	Engine::Material cube_material;
	Engine::Material cube_reflected_material;

	Engine::Material ground_material;
	Engine::Material wall_material;

	Engine::Material window_material;

	Engine::Material outline_material;

	Engine::Material mirror_quad_material;

	Engine::Material sphere_material;

/* Instancing Data: */

	struct LightInstanceData
	{
		Matrix4x4 transform;
		Engine::Color4 color;
	};

	std::vector< Matrix4x4 > cube_instance_data_array;
	std::vector< Matrix4x4 > cube_reflected_instance_data_array;
	std::vector< LightInstanceData > light_source_instance_data_array;

/* Scene: */
	/* Lights: */
	Engine::Transform light_directional_transform;
	std::vector< Engine::Transform > light_point_transform_array;
	Engine::Transform light_spot_transform;

	/* GameObjects: */
	std::vector< Engine::Transform > cube_transform_array;
	std::vector< Engine::Transform > cube_reflected_transform_array;

	Engine::Transform cube_parallax_transform;
	Engine::Transform ground_transform;
	Engine::Transform wall_front_transform;
	Engine::Transform wall_left_transform;
	Engine::Transform wall_right_transform;
	Engine::Transform wall_back_transform;

	Engine::Transform sphere_transform;

	std::array< Engine::Transform, WINDOW_COUNT > window_transform_array;

/* Lighting: */
	const static constexpr int LIGHT_POINT_COUNT = 15;

	Engine::MaterialData::BlinnPhongMaterialData cube_surface_data;
	Engine::MaterialData::BlinnPhongMaterialData ground_quad_surface_data;
	Engine::MaterialData::BlinnPhongMaterialData wall_surface_data;
	std::vector< Engine::MaterialData::BlinnPhongMaterialData > test_model_node_surface_data_array;

	Engine::DirectionalLight light_directional;
	std::vector< Engine::PointLight > light_point_array;
	Engine::SpotLight light_spot;

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

	bool render_rear_view_cam_to_imgui;
};