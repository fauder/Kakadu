#define IMGUI_DEFINE_MATH_OPERATORS

// HDR-Demo Includes.
#include "HDR_DemoApplication.h"

// Engine Includes.
#include "Engine/Asset/Shader/_Attributes.glsl"
#include "Engine/Core/AssetDatabase.hpp"
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiUtility.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Graphics/GLLogger.h"
#include "Engine/Graphics/BuiltinShaders.h"
#include "Engine/Graphics/BuiltinTextures.h"
#include "Engine/Graphics/Primitive/Primitive_Cube.h"
#include "Engine/Graphics/Primitive/Primitive_Sphere.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/VectorConversion.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.

#define AssetDir "../Common/Asset/Texture/"

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
{
	return new HDR_DemoApplication( flags );
}

HDR_DemoApplication::HDR_DemoApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
	:
	Engine::Application( flags,
						 Engine::Renderer::Description
						 {
							 .main_framebuffer_color_format      = Engine::Texture::Format::RGBA_16F,
							 .main_framebuffer_msaa_sample_count = 4
						 } ),
	light_point_transform_array( LIGHT_POINT_COUNT )
{
	Initialize();
}

HDR_DemoApplication::~HDR_DemoApplication()
{
	Shutdown();
}

void HDR_DemoApplication::Initialize()
{
	Platform::ChangeTitle( "Kakadu - HDR-Demo" );

	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "HDR-Demo GL Init." ) );

/* Textures: */
	wood_diffuse_map = Engine::ServiceLocator< Engine::AssetDatabase< Engine::Texture > >::Get().CreateAssetFromFile( "Wood (Diffuse) Map", AssetDir R"(wood.png)",
																													  Engine::Texture::ImportSettings
																													  {
																														  .wrap_u = Engine::Texture::Wrapping::Repeat,
																														  .wrap_v = Engine::Texture::Wrapping::Repeat
																													  } );

/* Shaders: */
	shader_blinn_phong           = Engine::BuiltinShaders::Get( "Blinn-Phong" );
	shader_basic_color_instanced = Engine::BuiltinShaders::Get( "Color (Instanced)" );
	shader_texture_blit          = Engine::BuiltinShaders::Get( "Texture Blit" );

/* Instance Data: */
	ResetInstanceData();

/* Lighting: */
	ResetLightingData(); // Also sets light transforms.

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer->AddPointLight( &light_point_array[ index ] );

/* Initial transforms: */
	tunnel_transform
		.SetScaling( 5.0f, 5.0f, 55.0f )
		.SetTranslation( 0.0f, 0.0f, 25.0f );

	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		light_source_instance_data_array[ i ].transform = light_point_transform_array[ i ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.
		light_source_instance_data_array[ i ].color     = Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
	}

/* Vertex/Index Data: */
	constexpr auto InvertAttributeArray = []( auto&& attribute_array )
	{
		auto copy( attribute_array );
		std::for_each( copy.begin(), copy.end(), []( auto&& element ) { element *= -1.0f; } );
		return copy;
	};

	constexpr auto cube_tangents_inverted = InvertAttributeArray( Engine::Primitive::Indexed::Cube::Tangents );
	constexpr auto cube_normals_inverted  = InvertAttributeArray( Engine::Primitive::Indexed::Cube::Normals );

	cube_mesh_inverted = Engine::Mesh( Engine::Primitive::Indexed::Cube::Positions,
									   "Cube (Inverted)",
									   cube_normals_inverted,
									   Engine::Primitive::Indexed::Cube::UVs,
									   Engine::Primitive::Indexed::Cube::Indices,
									   cube_tangents_inverted );

	const auto sphere_mesh = Engine::Mesh( Engine::Primitive::Indexed::Sphere::Positions(),
										   "Sphere",
										   Engine::Primitive::Indexed::Sphere::Normals(),
										   Engine::Primitive::Indexed::Sphere::UVs(),
										   Engine::Primitive::Indexed::Sphere::Indices(),
										   Engine::Primitive::Indexed::Sphere::Tangents() );

	light_source_sphere_mesh = Engine::Mesh( sphere_mesh,
											 {
												 Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4, INSTANCED_ATTRIBUTE_START },    // Transform.
												 Engine::VertexInstanceAttribute{ 1, GL_FLOAT_VEC4, INSTANCED_ATTRIBUTE_START + 4 } // Color.
											 },
											 reinterpret_cast< std::vector< float >& >( light_source_instance_data_array ),
											 LIGHT_POINT_COUNT,
											 GL_STATIC_DRAW );

	light_source_sphere_mesh.UpdateInstanceData( light_source_instance_data_array.data() );

/* Materials: */
	ResetMaterialData();

/* Renderer: */
	renderer->AddQueue( RENDER_QUEUE_ID_CUSTOM,
						Engine::RenderQueue
						{
						    .name = "Custom (Inverted)",
						    .render_state_override = Engine::RenderState
						    {
							    .face_culling_face_to_cull = Engine::Face::Front
						    }
						} );

	renderer->AddQueueToPass( RENDER_QUEUE_ID_CUSTOM, Engine::Renderer::RENDER_PASS_ID_LIGHTING );

	tunnel_renderable = Engine::Renderable( &cube_mesh_inverted, &wood_material, &tunnel_transform );
	renderer->AddRenderable( &tunnel_renderable, RENDER_QUEUE_ID_CUSTOM );

	light_sources_renderable = Engine::Renderable( &light_source_sphere_mesh, &light_source_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer->AddRenderable( &light_sources_renderable, Engine::Renderer::RENDER_QUEUE_ID_GEOMETRY );

	/* Disable some RenderPasses & Renderables on start-up to decrease clutter. */
	renderer->TogglePass( Engine::Renderer::RENDER_PASS_ID_SHADOW_MAPPING, false ); // No shadows necessary for this demo.
	renderer->ToggleQueue( Engine::Renderer::RENDER_QUEUE_ID_TRANSPARENT, false );

/* Camera: */
	//ResetCamera(); // TODO: Game camera rendering.

	/* This is the earliest place we can MaximizeWindow() at,
	 * because the Renderer will populate its Intrinsic UBO info only upon AddRenderable( <Renderable with a Shader using said UBO> ). */

	/* No need to Initialize Framebuffer related stuff as maximizing the window will cause them to be (re)initialized in OnFramebufferResizeEvent(). */

	Platform::MaximizeWindow();
}

void HDR_DemoApplication::Shutdown()
{
}

//void HDR_DemoApplication::Run()
//{
//
//}

void HDR_DemoApplication::Update()
{
	Application::Update();

	auto log_group( gl_logger.TemporaryLogGroup( "HDR-Demo Update()" ) );

	// TODO: Separate applicationg logs from GL logs.

	current_time_as_angle = Radians( time_current );
	const Radians current_time_mod_two_pi( std::fmod( time_current, Engine::Constants< float >::Two_Pi() ) );
}

void HDR_DemoApplication::RenderFrame()
{
	Engine::Application::RenderFrame();

	// Scene-view Camera moved into Application.

	//renderer->Render();
}

void HDR_DemoApplication::RenderImGui()
{
	/* Reminder: The rest of the rendering code (namely, ImGui) will be working in sRGB for the remainder of this frame,
	 * as the last step in the application's rendering was to enable sRGB encoding for the final framebuffer (default framebuffer or the final FBO). */

	/* Need to switch to the default framebuffer, so ImGui can render onto it. */
	renderer->ResetToDefaultFramebuffer();

	Application::RenderImGui();

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	const auto& style = ImGui::GetStyle();

	Engine::ImGuiDrawer::Draw( wood_material, *renderer );
	Engine::ImGuiDrawer::Draw( light_source_material, *renderer );

	if( ImGui::Begin( ICON_FA_LIGHTBULB " Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Checkbox( "Enabled##AllLights", &light_is_enabled ) )
		{
			for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
			{
				light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
			}
		}

		ImGui::SameLine( 0.0f, 20.0f );

		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Lights" ) )
			ResetLightingData();

		ImGui::NewLine();

		if( ImGui::BeginTabBar( "Lights Tab Bar", ImGuiTabBarFlags_DrawSelectedOverline | ImGuiTabBarFlags_NoTabListScrollingButtons ) )
		{
			if( ImGui::BeginTabItem( "Point Lights" ) )
			{
				Engine::ImGuiUtility::BeginGroupPanel( "Options" );
				if( ImGui::Checkbox( "Disable All", &light_point_array_disable ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
					}
				}
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					bool update_light_instance_buffer = false;
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str() ) )
						{
							light_source_instance_data_array[ i ].transform = light_point_array[ i ].transform->GetFinalMatrix().Transposed();
							update_light_instance_buffer = true;
						}

						light_source_sphere_mesh.UpdateInstanceData( light_source_instance_data_array.data() );
					}

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void HDR_DemoApplication::OnMouseButtonEvent( const Platform::MouseButton button, const Platform::MouseButtonAction button_action, const Platform::KeyMods key_mods )
{
	Application::OnMouseButtonEvent( button, button_action, key_mods );
}

void HDR_DemoApplication::OnMouseScrollEvent( const float x_offset, const float y_offset )
{
	Application::OnMouseScrollEvent( x_offset, y_offset );
}

void HDR_DemoApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
{
	/*switch( key_code )
	{
		default:
			break;
	}*/

	Application::OnKeyboardEvent( key_code, key_action, key_mods );
}

void HDR_DemoApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
}

void HDR_DemoApplication::ResetInstanceData()
{
	light_source_instance_data_array.resize( LIGHT_POINT_COUNT );
}

void HDR_DemoApplication::ResetLightingData()
{
	light_is_enabled = true;

	light_point_array_disable = false;

	light_point_transform_array[ 0 ] = Engine::Transform( Vector3::One(), Vector3(  0.0f,  0.0f, +49.5f ) );
	light_point_transform_array[ 1 ] = Engine::Transform( Vector3::One(), Vector3(  1.4f, -1.9f,  +9.0f ) );
	light_point_transform_array[ 2 ] = Engine::Transform( Vector3::One(), Vector3(  0.0f, -1.8f,  +4.0f ) );
	light_point_transform_array[ 3 ] = Engine::Transform( Vector3::One(), Vector3( -0.8f, -1.7f,  +6.0f ) );

	light_point_array.resize( LIGHT_POINT_COUNT );
	light_point_array[ 0 ] =
	{
		.is_enabled = true,
		.data =
		{
			.ambient_and_attenuation_constant = {.color = {},										.scalar = 0.0f },
			.diffuse_and_attenuation_linear   = {.color = Engine::Color3( 200.0f, 200.0f, 200.0f ), .scalar = 0.0f },
			.specular_attenuation_quadratic   = {.color = {},										.scalar = 1.0f },
		},
		.transform = &light_point_transform_array[ 0 ]
	};
	light_point_array[ 1 ] =
	{
		.is_enabled = true,
		.data =
		{
			.ambient_and_attenuation_constant = {.color = {},									.scalar = 0.0f },
			.diffuse_and_attenuation_linear   = {.color = Engine::Color3( 0.1f, 0.0f, 0.0f ),	.scalar = 0.0f },
			.specular_attenuation_quadratic   = {.color = {},									.scalar = 1.0f },
		},
		.transform = &light_point_transform_array[ 1 ]
	};
	light_point_array[ 2 ] =
	{
		.is_enabled = true,
		.data =
		{
			.ambient_and_attenuation_constant = {.color = {},									.scalar = 0.0f },
			.diffuse_and_attenuation_linear   = {.color = Engine::Color3( 0.0f, 0.0f, 0.2f ),	.scalar = 0.0f },
			.specular_attenuation_quadratic   = {.color = {},									.scalar = 1.0f },
		},
		.transform = &light_point_transform_array[ 2 ]
	};
	light_point_array[ 3 ] =
	{
		.is_enabled = true,
		.data =
		{
			.ambient_and_attenuation_constant = {.color = {},									.scalar = 0.0f },
			.diffuse_and_attenuation_linear   = {.color = Engine::Color3( 0.0f, 0.1f, 0.0f ),	.scalar = 0.0f },
			.specular_attenuation_quadratic   = {.color = {},									.scalar = 1.0f },
		},
		.transform = &light_point_transform_array[ 3 ]
	};
}

void HDR_DemoApplication::ResetMaterialData()
{
	wood_material = Engine::Material( "Wood", shader_blinn_phong );
	wood_material.SetTexture( "uniform_tex_diffuse", wood_diffuse_map );
	wood_material.SetTexture( "uniform_tex_specular", Engine::ServiceLocator< Engine::BuiltinTextures >::Get().Get( "White" ) );
	wood_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	light_source_material = Engine::Material( "Light Source", shader_basic_color_instanced );

	tunnel_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	wood_material.Set( "BlinnPhongMaterialData", tunnel_surface_data );
}
