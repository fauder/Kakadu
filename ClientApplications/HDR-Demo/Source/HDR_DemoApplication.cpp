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
#include "Engine/Graphics/RHI/Usage.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/VectorConversion.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.

#define AssetDir "../Common/Asset/Texture/"

using namespace Kakadu::Math::Literals;

Kakadu::Application* Kakadu::CreateApplication( const Kakadu::BitFlags< Kakadu::CreationFlags > flags )
{
	return new HDR_DemoApplication( flags );
}

HDR_DemoApplication::HDR_DemoApplication( const Kakadu::BitFlags< Kakadu::CreationFlags > flags )
	:
	Kakadu::Application( flags,
						 Kakadu::Renderer::Description
						 {
							 .main_framebuffer_color_format = Kakadu::Texture::Format::RGBA_16F,
							 .msaa_sample_count             = 4
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
	Kakadu::Platform::ChangeTitle( "Kakadu - HDR-Demo" );

	//Kakadu::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "HDR-Demo GL Init." ) );

/* Textures: */
	wood_diffuse_map = Kakadu::ServiceLocator< Kakadu::AssetDatabase< Kakadu::Texture > >::Get().CreateAssetFromFile( "Wood (Diffuse) Map", AssetDir R"(wood.png)",
																													  Kakadu::Texture::ImportSettings
																													  {
																														  .wrap_u = Kakadu::Texture::Wrapping::Repeat,
																														  .wrap_v = Kakadu::Texture::Wrapping::Repeat
																													  } );

/* Shaders: */
	shader_blinn_phong           = Kakadu::BuiltinShaders::Get( "Blinn-Phong" );
	shader_basic_color_instanced = Kakadu::BuiltinShaders::Get( "Color (Instanced)" );
	shader_texture_blit          = Kakadu::BuiltinShaders::Get( "Texture Blit" );

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
		light_source_instance_data_array[ i ].color     = Kakadu::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
	}

/* Vertex/Index Data: */
	constexpr auto InvertAttributeArray = []( auto&& attribute_array )
	{
		auto copy( attribute_array );
		std::for_each( copy.begin(), copy.end(), []( auto&& element ) { element *= -1.0f; } );
		return copy;
	};

	constexpr auto cube_tangents_inverted = InvertAttributeArray( Kakadu::Primitive::Indexed::Cube::Tangents );
	constexpr auto cube_normals_inverted  = InvertAttributeArray( Kakadu::Primitive::Indexed::Cube::Normals );

	cube_mesh_inverted = Kakadu::Mesh( Kakadu::Primitive::Indexed::Cube::Positions,
									   "Cube (Inverted)",
									   cube_normals_inverted,
									   Kakadu::Primitive::Indexed::Cube::UVs,
									   Kakadu::Primitive::Indexed::Cube::Indices,
									   cube_tangents_inverted );

	const auto sphere_mesh = Kakadu::Mesh( Kakadu::Primitive::Indexed::Sphere::Positions(),
										   "Sphere",
										   Kakadu::Primitive::Indexed::Sphere::Normals(),
										   Kakadu::Primitive::Indexed::Sphere::UVs(),
										   Kakadu::Primitive::Indexed::Sphere::Indices(),
										   Kakadu::Primitive::Indexed::Sphere::Tangents() );

	light_source_sphere_mesh = Kakadu::Mesh( sphere_mesh,
											 {
												 Kakadu::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4, INSTANCED_ATTRIBUTE_START },    // Transform.
												 Kakadu::VertexInstanceAttribute{ 1, GL_FLOAT_VEC4, INSTANCED_ATTRIBUTE_START + 4 } // Color.
											 },
											 reinterpret_cast< std::vector< float >& >( light_source_instance_data_array ),
											 LIGHT_POINT_COUNT,
											 Kakadu::RHI::Usage::StaticDraw );

	light_source_sphere_mesh.UpdateInstanceData( light_source_instance_data_array.data() );

/* Materials: */
	ResetMaterialData();

/* Renderer: */
	renderer->AddQueue( RENDER_QUEUE_ID_CUSTOM,
						Kakadu::RenderQueue
						{
						    .name = "Custom (Inverted)",
						    .render_state_override = Kakadu::RenderState
						    {
							    .face_culling_face_to_cull = Kakadu::Face::Front
						    }
						} );

	renderer->AddQueueToPass( RENDER_QUEUE_ID_CUSTOM, Kakadu::Renderer::RENDER_PASS_ID_LIGHTING );

	tunnel_renderable = Kakadu::Renderable( &cube_mesh_inverted, &wood_material, &tunnel_transform );
	renderer->AddRenderable( &tunnel_renderable, RENDER_QUEUE_ID_CUSTOM );

	light_sources_renderable = Kakadu::Renderable( &light_source_sphere_mesh, &light_source_material, nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer->AddRenderable( &light_sources_renderable, Kakadu::Renderer::RENDER_QUEUE_ID_GEOMETRY );

	/* Disable some RenderPasses & Renderables on start-up to decrease clutter. */
	renderer->TogglePass( Kakadu::Renderer::RENDER_PASS_ID_SHADOW_MAPPING, false ); // No shadows necessary for this demo.
	renderer->ToggleQueue( Kakadu::Renderer::RENDER_QUEUE_ID_TRANSPARENT, false );

/* Camera: */
	//ResetCamera(); // TODO: Game camera rendering.
}

void HDR_DemoApplication::Shutdown()
{
}

void HDR_DemoApplication::Update()
{
	Application::Update();

	auto log_group( gl_logger.TemporaryLogGroup( "HDR-Demo Update()" ) );

	// TODO: Separate application logs from GL logs.

	current_time_as_angle = Radians( frame_time.time_current );
	const Radians current_time_mod_two_pi( std::fmod( frame_time.time_current, Kakadu::Constants< float >::Two_Pi() ) );
}

void HDR_DemoApplication::RenderFrame()
{
	Kakadu::Application::RenderFrame();

	// Scene-view Camera moved into Application.

	//renderer->Render();
}

void HDR_DemoApplication::RenderToolsUI()
{
	/* Reminder: The rest of the rendering code (namely, ImGui) will be working in sRGB for the remainder of this frame,
	 * as the last step in the application's rendering was to enable sRGB encoding for the final framebuffer (default framebuffer or the final FBO). */

	/* Need to switch to the default framebuffer, so ImGui can render onto it. */
	renderer->ResetToDefaultFramebuffer();

	const auto& style = ImGui::GetStyle();

	Kakadu::ImGuiDrawer::Draw( wood_material, *renderer );
	Kakadu::ImGuiDrawer::Draw( light_source_material, *renderer );

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
				Kakadu::ImGuiUtility::BeginGroupPanel( "Options" );
				if( ImGui::Checkbox( "Disable All", &light_point_array_disable ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
					}
				}
				Kakadu::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					bool update_light_instance_buffer = false;
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Kakadu::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str() ) )
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

void HDR_DemoApplication::OnMouseButtonEvent( const Kakadu::Platform::MouseButton button, const Kakadu::Platform::MouseButtonAction button_action, const Kakadu::Platform::KeyMods key_mods )
{
	Application::OnMouseButtonEvent( button, button_action, key_mods );
}

void HDR_DemoApplication::OnMouseScrollEvent( const float x_offset, const float y_offset )
{
	Application::OnMouseScrollEvent( x_offset, y_offset );
}

void HDR_DemoApplication::OnKeyboardEvent( const Kakadu::Platform::KeyCode key_code, const Kakadu::Platform::KeyAction key_action, const Kakadu::Platform::KeyMods key_mods )
{
	/*switch( key_code )
	{
		default:
			break;
	}*/

	Application::OnKeyboardEvent( key_code, key_action, key_mods );
}

void HDR_DemoApplication::OnFramebufferResizeEvent( const i32 width_new_pixels, const i32 height_new_pixels )
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

	light_point_transform_array[ 0 ] = Kakadu::Transform( Vector3::One(), Vector3(  0.0f,  0.0f, +49.5f ) );
	light_point_transform_array[ 1 ] = Kakadu::Transform( Vector3::One(), Vector3(  1.4f, -1.9f,  +9.0f ) );
	light_point_transform_array[ 2 ] = Kakadu::Transform( Vector3::One(), Vector3(  0.0f, -1.8f,  +4.0f ) );
	light_point_transform_array[ 3 ] = Kakadu::Transform( Vector3::One(), Vector3( -0.8f, -1.7f,  +6.0f ) );

	light_point_array.resize( LIGHT_POINT_COUNT );
	light_point_array[ 0 ] =
	{
		.is_enabled = true,
		.data =
		{
			.ambient_and_attenuation_constant = {.color = {},										.scalar = 0.0f },
			.diffuse_and_attenuation_linear   = {.color = Kakadu::Color3( 200.0f, 200.0f, 200.0f ), .scalar = 0.0f },
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
			.diffuse_and_attenuation_linear   = {.color = Kakadu::Color3( 0.1f, 0.0f, 0.0f ),	.scalar = 0.0f },
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
			.diffuse_and_attenuation_linear   = {.color = Kakadu::Color3( 0.0f, 0.0f, 0.2f ),	.scalar = 0.0f },
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
			.diffuse_and_attenuation_linear   = {.color = Kakadu::Color3( 0.0f, 0.1f, 0.0f ),	.scalar = 0.0f },
			.specular_attenuation_quadratic   = {.color = {},									.scalar = 1.0f },
		},
		.transform = &light_point_transform_array[ 3 ]
	};
}

void HDR_DemoApplication::ResetMaterialData()
{
	wood_material = Kakadu::Material( "Wood", shader_blinn_phong );
	wood_material.SetTexture( "uniform_tex_diffuse", wood_diffuse_map );
	wood_material.SetTexture( "uniform_tex_specular", Kakadu::ServiceLocator< Kakadu::BuiltinTextures >::Get().Get( "White" ) );
	wood_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	light_source_material = Kakadu::Material( "Light Source", shader_basic_color_instanced );

	tunnel_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	wood_material.Set( "BlinnPhongMaterialData", tunnel_surface_data );
}
