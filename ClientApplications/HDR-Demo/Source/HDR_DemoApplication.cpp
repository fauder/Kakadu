#define IMGUI_DEFINE_MATH_OPERATORS

// Sandbox Includes.
#include "HDR_DemoApplication.h"

// Engine Includes.
#include "Engine/Asset/Shader/_Attributes.glsl"
#include "Engine/Core/AssetDatabase.hpp"
#include "Engine/Core/ImGuiDrawer.hpp"
#include "Engine/Core/ImGuiSetup.h"
#include "Engine/Core/ImGuiUtility.h"
#include "Engine/Core/Platform.h"
#include "Engine/Core/ServiceLocator.h"
#include "Engine/Graphics/GLLogger.h"
#include "Engine/Graphics/InternalShaders.h"
#include "Engine/Graphics/InternalTextures.h"
#include "Engine/Graphics/MeshUtility.hpp"
#include "Engine/Graphics/Primitive/Primitive_Cube.h"
#include "Engine/Graphics/Primitive/Primitive_Cube_FullScreen.h"
#include "Engine/Graphics/Primitive/Primitive_Sphere.h"
#include "Engine/Graphics/Primitive/Primitive_Quad.h"
#include "Engine/Graphics/Primitive/Primitive_Quad_FullScreen.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"
#include "Engine/Math/VectorConversion.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <execution>
#include <fstream>

#define AssetDir "../Common/Asset/Texture/"

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
{
	return new HDR_DemoApplication( flags );
}

HDR_DemoApplication::HDR_DemoApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
	:
	Engine::Application( flags ),
	renderer( /* Offscreen framebuffer MSAA sample counts: */{ /* Main: */ 4, /* Rear-view: */ std::nullopt } ),
	light_point_transform_array( LIGHT_POINT_COUNT ),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two(), Platform::GetAspectRatio() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
	ui_interaction_enabled( false ),
	show_imgui_demo_window( false )
{
	Initialize();
}

HDR_DemoApplication::~HDR_DemoApplication()
{
	Shutdown();
}

void HDR_DemoApplication::Initialize()
{
	Platform::ChangeTitle( "Kakadu - Sandbox" );

	gl_logger.IgnoreID( 131185 );

	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox GL Init." ) );

/* Textures: */
	wood_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Wood (Diffuse) Map", AssetDir R"(wood.png)" );

/* Shaders: */
	shader_blinn_phong             = Engine::InternalShaders::Get( "Blinn-Phong" );

	shader_texture_blit            = Engine::InternalShaders::Get( "Texture Blit" );
	shader_fullscreen_blit         = Engine::InternalShaders::Get( "Fullscreen Blit" );
	shader_fullscreen_blit_resolve = Engine::InternalShaders::Get( "Fullscreen Blit Resolve" );

/* Initial transforms: */
	tunnel_transform
		.SetScaling( 2.5f, 2.5f, 27.5f )
		.SetTranslation( 0.0f, 0.0f, 25.0f );

/* Vertex/Index Data: */
	cube_mesh = Engine::Mesh( Engine::Primitive::Indexed::Cube::Positions,
							  "Cube",
							  Engine::Primitive::Indexed::Cube::Normals,
							  Engine::Primitive::Indexed::Cube::UVs,
							  Engine::Primitive::Indexed::Cube::Indices,
							  Engine::Primitive::Indexed::Cube::Tangents );

	quad_mesh_fullscreen = Engine::Mesh( Engine::Primitive::NonIndexed::Quad_FullScreen::Positions,
										 "Quad (FullScreen)",
										 { /* No normals. */ },
										 Engine::Primitive::NonIndexed::Quad_FullScreen::UVs,
										 { /* No indices. */ } );

/* Lighting: */
	ResetLightingData();

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer.AddPointLight( &light_point_array[ index ] );

/* Materials: */
	ResetMaterialData();

/* Renderer: */
	renderer.AddQueue( RENDER_QUEUE_ID_GEOMETRY_INVERTED, Engine::RenderQueue
					   {
						   .name                  = "Geometry Inverted",
						   .render_state_override = Engine::RenderState
							{
								.face_culling_face_to_cull = Engine::Face::Front
							}
					   } );

	renderer.AddQueueToPass( RENDER_QUEUE_ID_GEOMETRY_INVERTED, Engine::Renderer::PASS_ID_LIGHTING );

	tunnel_renderable = Engine::Renderable( &cube_mesh, &tunnel_material, &tunnel_transform );
	renderer.AddRenderable( &tunnel_renderable, RENDER_QUEUE_ID_GEOMETRY_INVERTED );

	offscreen_quad_renderable = Engine::Renderable( &quad_mesh_fullscreen, &offscreen_quad_material );
	renderer.AddRenderable( &offscreen_quad_renderable, Engine::Renderer::QUEUE_ID_POSTPROCESSING );

	// TODO: Do not create an explicit (or rather, Application-visible) Renderable for skybox; Make it Renderer-internal.

	/* Disable some RenderPasses & Renderables on start-up to decrease clutter. */
	renderer.TogglePass( Engine::Renderer::PASS_ID_OUTLINE, false );
	renderer.ToggleQueue( Engine::Renderer::QUEUE_ID_TRANSPARENT, false );

/* Camera: */
	ResetCamera();

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
	auto log_group( gl_logger.TemporaryLogGroup( "Sandbox Update()" ) );

	// TODO: Separate applicationg logs from GL logs.

	current_time_as_angle = Radians( time_current );
	const Radians current_time_mod_two_pi( std::fmod( time_current, Engine::Constants< float >::Two_Pi() ) );

	/* Camera transform: */
	{
		if( !ui_interaction_enabled )
		{
			// Control via mouse:
			const auto [ mouse_x_delta_pos, mouse_y_delta_pos ] = Platform::GetMouseCursorDeltas();
			camera_controller
				.OffsetHeading( Radians( +mouse_x_delta_pos ) )
				.OffsetPitch( Radians( +mouse_y_delta_pos ), -( Engine::Constants< Radians >::Pi_Over_Two() - 0.01_rad ), Engine::Constants< Radians >::Pi_Over_Two() - 0.01_rad );
		}
	}

	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_W ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_S ) )
		camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_A ) )
		camera_transform.OffsetTranslation( camera_transform.Right() * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_D ) )
		camera_transform.OffsetTranslation( camera_transform.Right() * +camera_move_speed * time_delta );

	renderer.Update();
}

void HDR_DemoApplication::Render()
{
	Engine::Application::Render();

	/* Lighting: Render everything to the off-screen framebuffer 1: */
	{
		renderer.UpdatePerPass( Engine::Renderer::PASS_ID_LIGHTING, camera );
	}

	// TODO: Outline pass.

	/* Post-processing pass: Blit off-screen framebuffers to quads on the default or the editor framebuffer to actually display them: */
	{
		/* This pass does not utilize camera view/projection => no Renderer::Update() necessary. */
	}

	renderer.Render();

}

void HDR_DemoApplication::RenderImGui()
{
	/* Reminder: The rest of the rendering code (namely, ImGui) will be working in sRGB for the remainder of this frame,
	 * as the last step in the application's rendering was to enable sRGB encoding for the final framebuffer (default framebuffer or the editor FBO). */

	/* Need to switch to the default framebuffer, so ImGui can render onto it. */
	renderer.ResetToDefaultFramebuffer();

	Application::RenderImGui();

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	RenderImGui_Viewport();

	const auto& style = ImGui::GetStyle();

	Engine::ImGuiDrawer::Draw( tunnel_material, renderer );
	Engine::ImGuiDrawer::Draw( offscreen_quad_material, renderer );

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
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str() ) )
						{
						}
					}

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_VIDEO " Camera", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
	/* Row 1: */
		const float button_width( ImGui::CalcTextSize( ICON_FA_ARROWS_ROTATE " XXXXXX" ).x + style.ItemInnerSpacing.x );
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Top" ) )
			SwitchCameraView( CameraView::TOP );

	/* Row 2: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Left" ) )
			SwitchCameraView( CameraView::LEFT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Front" ) )
			SwitchCameraView( CameraView::FRONT );
		ImGui::SameLine();
		ImGui::SetCursorPosX( button_width * 2 );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Right" ) )
			SwitchCameraView( CameraView::RIGHT );

	/* Row 3: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Back" ) )
			SwitchCameraView( CameraView::BACK );

	/* Row 4: */
		ImGui::SetCursorPosX( button_width );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Bottom" ) )
			SwitchCameraView( CameraView::BOTTOM );


		ImGui::NewLine();

	/* Row 5: */
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Custom (1)" ) )
			SwitchCameraView( CameraView::CUSTOM_1 );
		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Camera" ) )
			ResetCamera();

		Engine::ImGuiDrawer::Draw( camera_transform, Engine::Transform::Mask::NoScale, "Main Camera" );
	}

	ImGui::End();

	if( ImGui::Begin( "Projection", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		Engine::ImGuiUtility::BeginGroupPanel();
		{
			if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Projection" ) )
				ResetProjection();

			if( Engine::ImGuiDrawer::Draw( camera, "Main Camera", true ) )
				RecalculateProjectionParameters();
		}
		Engine::ImGuiUtility::EndGroupPanel();
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( Engine::AssetDatabase< Engine::Texture >::Assets(), { 400.0f, 512.0f } );

	renderer.RenderImGui();
}

void HDR_DemoApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
{
	switch( key_code )
	{
		case Platform::KeyCode::KEY_ESCAPE:
			if( key_action == Platform::KeyAction::PRESS )
				Platform::SetShouldClose( true );
			break;
		/* Use the key below ESC to toggle between game & menu/UI. */
		case Platform::KeyCode::KEY_GRAVE_ACCENT:
			if( key_action == Platform::KeyAction::PRESS )
			{
				ui_interaction_enabled = !ui_interaction_enabled;
				Platform::ResetMouseDeltas();
			}
			break;
		case Platform::KeyCode::KEY_W:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_S:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Forward() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_A:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * -camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_D:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				camera_transform.OffsetTranslation( camera_transform.Right() * +camera_move_speed * time_delta );
			break;
		case Platform::KeyCode::KEY_I:
			if( key_action == Platform::KeyAction::PRESS )
			{
				show_imgui = !show_imgui;
				if( show_imgui )
				{
					renderer.SetFinalPassToUseEditorFramebuffer();
				}
				else
				{
					renderer.SetFinalPassToUseDefaultFramebuffer();
					OnFramebufferResizeEvent( Platform::GetFramebufferSizeInPixels() );
				}
			}
			break;
		case Platform::KeyCode::KEY_O:
			if( key_action == Platform::KeyAction::PRESS )
				show_imgui_demo_window = !show_imgui_demo_window;
			break;
		default:
			break;
	}
}

void HDR_DemoApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
	/* Do nothing on minimize: */
	if( width_new_pixels == 0 || height_new_pixels == 0 ||
		( renderer.EditorFramebuffer().Size() == Vector2I{ width_new_pixels, height_new_pixels } ) )
		return;

	renderer.OnFramebufferResize( width_new_pixels, height_new_pixels );

	RecalculateProjectionParameters( width_new_pixels, height_new_pixels );

	// TODO: Move these into Renderer: Maybe Materials can have a sort of requirements info. (or dependencies) and the Renderer can automatically update Material info such as the ones below.

	offscreen_quad_material.SetTexture( "uniform_texture_slot", &renderer.OffscreenFramebuffer( 0 ).ColorAttachment() );
}

void HDR_DemoApplication::OnFramebufferResizeEvent( const Vector2I new_size_pixels )
{
	OnFramebufferResizeEvent( new_size_pixels.X(), new_size_pixels.Y() );
}

void HDR_DemoApplication::RenderImGui_Viewport()
{
	{
		const auto framebuffer_size = Platform::GetFramebufferSizeInPixels();
		ImGui::SetNextWindowSize( Engine::Math::CopyToImVec2( framebuffer_size ), ImGuiCond_Appearing );
	}

	if( ImGui::Begin( "Viewport" ) )
	{
		const ImVec2   viewport_size_imvec2( ImGui::GetContentRegionAvail() );
		const Vector2I viewport_size( ( int )viewport_size_imvec2.x, ( int )viewport_size_imvec2.y );

		const auto& imgui_io = ImGui::GetIO();
		if( ( imgui_io.WantCaptureMouse && imgui_io.MouseReleased[ 0 ] ) ||
			( not imgui_io.WantCaptureMouse && Platform::IsMouseButtonReleased( Platform::MouseButton::Left ) ) )
			OnFramebufferResizeEvent( viewport_size.X(), viewport_size.Y() );

		if( ImGui::IsWindowHovered() )
		{
			ImGui::SetNextFrameWantCaptureMouse( false );
			ImGui::SetNextFrameWantCaptureKeyboard( false );
		}

		ImGui::Image( ( void* )( intptr_t )renderer.EditorFramebuffer().ColorAttachment().Id().Get(), ImGui::GetContentRegionAvail(), { 0, 1 }, { 1, 0 } );
	}

	ImGui::End();
}

void HDR_DemoApplication::ResetLightingData()
{
	light_is_enabled = true;

	light_point_array_disable = false;

	light_point_transform_array[ 0 ] = Engine::Transform( Vector3::One(), Vector3(  0.0f,  0.0f, 49.5f ) );
	light_point_transform_array[ 1 ] = Engine::Transform( Vector3::One(), Vector3( -1.4f, -1.9f,  9.0f ) );
	light_point_transform_array[ 2 ] = Engine::Transform( Vector3::One(), Vector3(  0.0f, -1.8f,  4.0f ) );
	light_point_transform_array[ 3 ] = Engine::Transform( Vector3::One(), Vector3(  0.8f, -1.7f,  6.0f ) );

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
	tunnel_material = Engine::Material( "Tunnel", shader_blinn_phong );
	tunnel_material.SetTexture( "uniform_diffuse_map_slot", wood_diffuse_map );
	tunnel_material.SetTexture( "uniform_specular_map_slot", Engine::ServiceLocator< Engine::InternalTextures >::Get().Get( "White" ) );
	tunnel_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	/*if( const auto& main_offscreen_framebuffer = renderer.OffscreenFramebuffer( 0 );
		main_offscreen_framebuffer.IsMultiSampled() )*/
	{
		offscreen_quad_material = Engine::Material( "Offscreen Quad", shader_fullscreen_blit_resolve );
		//offscreen_quad_material.Set( "uniform_sample_count", main_offscreen_framebuffer.SampleCount() );
		// TODO: Get rid of the hard-coding here.
		offscreen_quad_material.Set( "uniform_sample_count", 4 );
	}
	/*else
		offscreen_quad_material = Engine::Material( "Offscreen Quad", shader_fullscreen_blit );*/

	tunnel_surface_data =
	{
		.color_diffuse = {},
		.has_texture_diffuse = 1,
		.shininess = 32.0f
	};

	tunnel_material.Set( "BlinnPhongMaterialData", tunnel_surface_data );
}

void HDR_DemoApplication::ResetCamera()
{
	ResetProjection();

	SwitchCameraView( CameraView::CUSTOM_1 );
}

void HDR_DemoApplication::ResetProjection()
{
	camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), camera.GetVerticalFieldOfView() ); // Keep current aspect ratio & v-fov.
}

void HDR_DemoApplication::SwitchCameraView( const CameraView view )
{
	switch( view )
	{
		case CameraView::FRONT:
			camera_transform.SetTranslation( 0.0f, 10.0f, -20.0f );
			camera_transform.LookAt( Vector3::Forward() );
			break;
		case CameraView::BACK:
			camera_transform.SetTranslation( 0.0f, 10.0f, +20.0f );
			camera_transform.LookAt( Vector3::Backward() );
			break;
		case CameraView::LEFT:
			camera_transform.SetTranslation( -10.0f, 10.0f, 0.0f );
			camera_transform.LookAt( Vector3::Right() );
			break;
		case CameraView::RIGHT:
			camera_transform.SetTranslation( +10.0f, 10.0f, 0.0f );
			camera_transform.LookAt( Vector3::Left() );
			break;
		case CameraView::TOP:
			camera_transform.SetTranslation( 0.0f, 60.0f, 0.0f );
			camera_transform.LookAt( Vector3::Down() );
			break;
		case CameraView::BOTTOM:
			camera_transform.SetTranslation( 0.0f, -20.0f, 0.0f );
			camera_transform.LookAt( Vector3::Up() );
			break;



		case CameraView::CUSTOM_1:
			camera_transform.SetTranslation( 0.0f, 0.0f, 9.9f );
			camera_transform.SetRotation( Quaternion::Identity() );
			break;

		default:
			break;
	}

	camera_controller.ResetToTransform();
}

HDR_DemoApplication::Radians HDR_DemoApplication::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view, const float aspect_ratio ) const
{
	return 2.0f * Engine::Math::Atan2( Engine::Math::Tan( horizontal_field_of_view / 2.0f ), aspect_ratio );
}

void HDR_DemoApplication::RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels )
{
	camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
	camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two(), camera.GetAspectRatio() ) );
}

void HDR_DemoApplication::RecalculateProjectionParameters( const Vector2I new_size_pixels )
{
	RecalculateProjectionParameters( new_size_pixels.X(), new_size_pixels.Y() );
}

void HDR_DemoApplication::RecalculateProjectionParameters()
{
	RecalculateProjectionParameters( renderer.EditorFramebuffer().Size() );
}
