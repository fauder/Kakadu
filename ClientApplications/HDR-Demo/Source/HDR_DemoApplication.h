// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/DefineMathTypes.h"

class HDR_DemoApplication : public Engine::Application
{
	DEFINE_MATH_TYPES()

public:
	HDR_DemoApplication( const Engine::BitFlags< Engine::CreationFlags > );
	virtual ~HDR_DemoApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	//virtual void Run() override;

	virtual void Update() override;

	virtual void Render() override;

	virtual void RenderImGui() override;
	virtual void OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods ) override;
	virtual void OnMouseScrollEvent( const float x_offset, const float y_offset ) override;
	virtual void OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;

private:
	void ResetInstanceData();
	void ResetLightingData();
	void ResetMaterialData();
	void RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels );

private:
/* Renderer: */
	static constexpr Engine::RenderQueue::ID RENDER_QUEUE_ID_CUSTOM = Engine::RenderQueue::ID( ( uint16_t )Engine::Renderer::RENDER_QUEUE_ID_GEOMETRY + 1u );

	Engine::Renderable tunnel_renderable;

	Engine::Renderable light_sources_renderable;

	Engine::Framebuffer framebuffer_hdr;

/* Textures: */
	Engine::Texture* wood_diffuse_map;

	Engine::Texture* framebuffer_hdr_color_attachment;
	Engine::Texture* framebuffer_hdr_depth_attachment;

/* Vertex Info.: */
	Engine::Mesh cube_mesh_inverted;
	Engine::Mesh light_source_sphere_mesh;

/* Shaders: */
	Engine::Shader* shader_blinn_phong;

	Engine::Shader* shader_basic_color_instanced;

	Engine::Shader* shader_texture_blit;

/* Materials: */
	Engine::Material wood_material;

	Engine::Material light_source_material;

/* Scene: */
	/* Lights: */
	std::vector< Engine::Transform > light_point_transform_array;

	/* GameObjects: */

	Engine::Transform tunnel_transform;

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
};