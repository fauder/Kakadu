// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Renderer.h"
#include "Engine/Graphics/Texture.h"
#include "Engine/Scene/Camera.h"
#include "Engine/Scene/CameraController_Flight.h"

#include "Engine/DefineMathTypes.h"

class HDR_DemoApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

	enum class CameraView
	{
		FRONT,
		BACK,
		LEFT,
		RIGHT,
		TOP,
		BOTTOM,

		CUSTOM_1,
	};

public:
	HDR_DemoApplication( const Engine::BitFlags< Engine::CreationFlags > );
	virtual ~HDR_DemoApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Update() override;

	virtual void Render() override;

	virtual void RenderImGui() override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;
	void OnFramebufferResizeEvent( const Vector2I new_size_pixels ); // Convenience overload.

private:
	void RenderImGui_Viewport();

	void ResetInstanceData();
	void ResetLightingData();
	void ResetMaterialData();
	void ResetCamera();
	void ResetProjection();
	void SwitchCameraView( const CameraView view );
	Radians CalculateVerticalFieldOfView( const Radians horizontal_field_of_view, const float aspect_ratio ) const;

	void RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels );
	void RecalculateProjectionParameters( const Vector2I new_size_pixels ); // Convenience overload.
	void RecalculateProjectionParameters(); // Utilizes current framebuffer size.

private:
/* Renderer: */
	Engine::Renderer renderer;

	static constexpr Engine::RenderQueue::ID QUEUE_ID_CUSTOM = Engine::RenderQueue::ID( ( uint16_t )Engine::Renderer::QUEUE_ID_GEOMETRY + 1u );

	Engine::Renderable tunnel_renderable;

	Engine::Renderable light_sources_renderable;

	Engine::Renderable offscreen_quad_renderable;

	Engine::Framebuffer framebuffer_hdr;

/* Textures: */
	Engine::Texture* wood_diffuse_map;

	Engine::Texture* framebuffer_hdr_color_attachment;
	Engine::Texture* framebuffer_hdr_depth_attachment;

/* Vertex Info.: */
	Engine::Mesh cube_mesh_inverted;
	Engine::Mesh quad_mesh_fullscreen;
	Engine::Mesh light_source_sphere_mesh;

/* Shaders: */
	Engine::Shader* shader_blinn_phong;

	Engine::Shader* shader_basic_color_instanced;

	Engine::Shader* shader_texture_blit;
	Engine::Shader* shader_fullscreen_blit_and_tonemap;
	Engine::Shader* shader_fullscreen_blit_resolve_and_tonemap;

/* Materials: */
	Engine::Material wood_material;

	Engine::Material light_source_material;

	Engine::Material offscreen_quad_material;

/* Scene: */
	Engine::Transform camera_transform;

	/* Lights: */
	std::vector< Engine::Transform > light_point_transform_array;

	/* GameObjects: */

	Engine::Transform tunnel_transform;

/* Camera: */
	Engine::Camera camera;
	float camera_rotation_speed;
	float camera_move_speed;
	Engine::CameraController_Flight camera_controller;

/* Instancing Data: */
	struct LightInstanceData
	{
		Matrix4x4 transform;
		Engine::Color4 color;
	};

	std::vector< LightInstanceData > light_source_instance_data_array;

/* Lighting: */
	const static constexpr int LIGHT_POINT_COUNT = 4;

	Engine::MaterialData::BlinnPhongMaterialData tunnel_surface_data;

	std::vector< Engine::PointLight > light_point_array;

	bool light_is_enabled;
	bool light_point_array_disable;

/* Frame statistics: */
	Radians current_time_as_angle;

/* Other: */
	bool ui_interaction_enabled;
	bool show_imgui_demo_window;
};