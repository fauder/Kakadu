// Engine Includes.
#include "Engine.h"
#include "Engine/Graphics/Lighting/Lighting.h"
#include "Engine/Graphics/MaterialData/MaterialData.h"
#include "Engine/Graphics/Material.hpp"
#include "Engine/Graphics/Mesh.h"
#include "Engine/Graphics/Texture.h"

#include "Engine/DefineMathTypes.h"

class HDR_DemoApplication : public Kakadu::Application
{
	DEFINE_MATH_TYPES()

public:
	HDR_DemoApplication( const Kakadu::BitFlags< Kakadu::CreationFlags > );
	virtual ~HDR_DemoApplication();

	virtual void Initialize() override;
	virtual void Shutdown() override;

	virtual void Update() override;

	virtual void RenderFrame() override;

	virtual void RenderToolsUI() override;

	virtual void OnMouseButtonEvent( const Kakadu::Platform::MouseButton button, const Kakadu::Platform::MouseButtonAction button_action, const Kakadu::Platform::KeyMods key_mods ) override;
	virtual void OnMouseScrollEvent( const float x_offset, const float y_offset ) override;
	virtual void OnKeyboardEvent( const Kakadu::Platform::KeyCode key_code, const Kakadu::Platform::KeyAction key_action, const Kakadu::Platform::KeyMods key_mods ) override;
	virtual void OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels ) override;

private:
	void ResetInstanceData();
	void ResetLightingData();
	void ResetMaterialData();
	void RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels );

private:
/* Renderer: */
	static constexpr Kakadu::RenderQueue::ID RENDER_QUEUE_ID_CUSTOM = Kakadu::RenderQueue::ID( ( uint16_t )Kakadu::Renderer::RENDER_QUEUE_ID_GEOMETRY + 1u );

	Kakadu::Renderable tunnel_renderable;

	Kakadu::Renderable light_sources_renderable;

	Kakadu::Framebuffer framebuffer_hdr;

/* Textures: */
	Kakadu::Texture* wood_diffuse_map;

	Kakadu::Texture* framebuffer_hdr_color_attachment;
	Kakadu::Texture* framebuffer_hdr_depth_attachment;

/* Vertex Info.: */
	Kakadu::Mesh cube_mesh_inverted;
	Kakadu::Mesh light_source_sphere_mesh;

/* Shaders: */
	Kakadu::Shader* shader_blinn_phong;

	Kakadu::Shader* shader_basic_color_instanced;

	Kakadu::Shader* shader_texture_blit;

/* Materials: */
	Kakadu::Material wood_material;

	Kakadu::Material light_source_material;

/* Scene: */
	/* Lights: */
	std::vector< Kakadu::Transform > light_point_transform_array;

	/* GameObjects: */

	Kakadu::Transform tunnel_transform;

/* Instancing Data: */
	struct LightInstanceData
	{
		Matrix4x4 transform;
		Kakadu::Color4 color;
	};

	std::vector< LightInstanceData > light_source_instance_data_array;

/* Lighting: */
	const static constexpr int LIGHT_POINT_COUNT = 4;

	Kakadu::MaterialData::BlinnPhongMaterialData tunnel_surface_data;

	std::vector< Kakadu::PointLight > light_point_array;

	bool light_is_enabled;
	bool light_point_array_disable;

/* Frame statistics: */
	Radians current_time_as_angle;
};