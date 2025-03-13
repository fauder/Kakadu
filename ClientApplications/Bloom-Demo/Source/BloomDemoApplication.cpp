#define IMGUI_DEFINE_MATH_OPERATORS

// Bloom Demo Includes.
#include "BloomDemoApplication.h"

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
#include "Engine/Graphics/Primitive/Primitive_Cube_FullScreen.h"
#include "Engine/Graphics/Primitive/Primitive_Sphere.h"
#include "Engine/Graphics/Primitive/Primitive_Quad.h"
#include "Engine/Graphics/Primitive/Primitive_Quad_FullScreen.h"
#include "Engine/Math/Math.hpp"
#include "Engine/Math/Matrix.h"
#include "Engine/Math/Random.hpp"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

// std Includes.
#include <execution>
#include <fstream>

#define AssetDir "../Common/Asset/Texture/"

using namespace Engine::Math::Literals;

Engine::Application* Engine::CreateApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
{
    return new BloomDemoApplication( flags );
}

BloomDemoApplication::BloomDemoApplication( const Engine::BitFlags< Engine::CreationFlags > flags )
	:
	Engine::Application( flags,
						 Engine::Renderer::Description
						 {
							 .main_framebuffer_color_format      = Engine::Texture::Format::RGBA_16F,
							 .main_framebuffer_msaa_sample_count = 4,
							 .custom_framebuffer_descriptions    =
							 {
								 Engine::Framebuffer::Description
								 {
									 .name            = "Rear-view Mirror FB",
									 .color_format    = Engine::Texture::Format::RGBA,
									 .attachment_bits = Engine::Framebuffer::AttachmentType::Color_DepthStencilCombined
								 }
							 }
						 } ),
	test_model_info
	{
		.model_instance          = {},
		.shader                  = Engine::BuiltinShaders::Get( "Blinn-Phong" ),
		.shader_shadow_receiving = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed)" ),
		.file_path               = {},
		.is_receiving_shadows    = true,
		.is_casting_shadows      = true
	},
	meteorite_model_info
	{
		.model_instance          = {},
		.shader                  = Engine::BuiltinShaders::Get( "Blinn-Phong (Instanced)" ),
		.shader_shadow_receiving = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed | Instanced)" ),
		.file_path               = {},
		.is_receiving_shadows    = true,
		.is_casting_shadows      = true
	},
	light_point_transform_array( LIGHT_POINT_COUNT ),
	cube_transform_array( CUBE_COUNT ),
	cube_reflected_transform_array( CUBE_REFLECTED_COUNT ),
	camera( &camera_transform, Platform::GetAspectRatio(), CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two(), Platform::GetAspectRatio() ) ),
	camera_rotation_speed( 5.0f ),
	camera_move_speed( 5.0f ),
	camera_controller( &camera, camera_rotation_speed ),
	ui_interaction_enabled( true ),
	show_imgui_demo_window( false )
{
	Initialize();
}

BloomDemoApplication::~BloomDemoApplication()	
{
	Shutdown();
}

void BloomDemoApplication::Initialize()
{
	Platform::ChangeTitle( "Kakadu - Bloom Demo" );
	
	//Engine::Math::Random::SeedRandom();

	auto log_group( gl_logger.TemporaryLogGroup( "Bloom Demo GL Init." ) );

/* Textures: */
	skybox_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Skybox", 
																					{
																						AssetDir R"(Skybox/right.jpg)",
																						AssetDir R"(Skybox/left.jpg)",
																						AssetDir R"(Skybox/top.jpg)",
																						AssetDir R"(Skybox/bottom.jpg)",
																						AssetDir R"(Skybox/front.jpg)",
																						AssetDir R"(Skybox/back.jpg)"
																					},
																					Engine::Texture::ImportSettings
																					{
																						.min_filter      = Engine::Texture::Filtering::Linear,
																						.flip_vertically = false,
																					} );
		
	container_texture_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Diffuse) Map",	AssetDir R"(container2.png)" );
	container_texture_specular_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Container (Specular) Map", AssetDir R"(container2_specular.png)" );

	brickwall_diffuse_map  = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Brickwall (Diffuse) Map", AssetDir R"(bricks2.jpg)",
																							Engine::Texture::ImportSettings
																							{
																								.wrap_u = Engine::Texture::Wrapping::Repeat,
																								.wrap_v = Engine::Texture::Wrapping::Repeat
																							} );
	brickwall_normal_map   = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Brickwall (Normal) Map",  AssetDir R"(bricks2_normal.jpg)",
																							Engine::Texture::ImportSettings
																							{
																								.wrap_u  = Engine::Texture::Wrapping::Repeat,
																								.wrap_v  = Engine::Texture::Wrapping::Repeat,
																								.format  = Engine::Texture::Format::RGBA,
																							} );

	brickwall_displacement_map = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Brickwall (Displacement) Map", AssetDir R"(bricks2_disp.jpg)",
																								Engine::Texture::ImportSettings
																								{
																									.format = Engine::Texture::Format::RGBA,
																								} );

	transparent_window_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Transparent Window", AssetDir R"(blending_transparent_window.png)" );
	
	checker_pattern_texture = Engine::AssetDatabase< Engine::Texture >::CreateAssetFromFile( "Checkerboard Pattern 09", AssetDir R"(kenney_prototype/texture_09.png)", 
																							 Engine::Texture::ImportSettings
																							 {
																								 .wrap_u = Engine::Texture::Wrapping::Repeat,
																								 .wrap_v = Engine::Texture::Wrapping::Repeat
																							 } );

/* Shaders: */
	shader_skybox                                           = Engine::BuiltinShaders::Get( "Skybox" );
	shader_blinn_phong                                      = Engine::BuiltinShaders::Get( "Blinn-Phong" );
	shader_blinn_phong_shadowed                             = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed)" );
	shader_blinn_phong_shadowed_parallax                    = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed | Parallax)" );
	shader_blinn_phong_instanced                            = Engine::BuiltinShaders::Get( "Blinn-Phong (Instanced)" );
	shader_blinn_phong_shadowed_instanced                   = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed | Instanced)" );
	shader_blinn_phong_shadowed_parallax_instanced          = Engine::BuiltinShaders::Get( "Blinn-Phong (Shadowed | Parallax | Instanced)" );
	shader_blinn_phong_skybox_reflection                    = Engine::BuiltinShaders::Get( "Blinn-Phong (Skybox Reflection)" );
	shader_blinn_phong_skybox_reflection_instanced          = Engine::BuiltinShaders::Get( "Blinn-Phong (Skybox Reflection | Instanced)" );
	shader_blinn_phong_skybox_reflection_shadowed_instanced = Engine::BuiltinShaders::Get( "Blinn-Phong (Skybox Reflection | Shadowed | Instanced)" );
	shader_basic_color                                      = Engine::BuiltinShaders::Get( "Color" );
	shader_basic_color_instanced                            = Engine::BuiltinShaders::Get( "Color (Instanced)" );
	shader_basic_textured                                   = Engine::BuiltinShaders::Get( "Textured" );
	shader_basic_textured_transparent_discard               = Engine::BuiltinShaders::Get( "Textured (Discard Transparent)" );
	shader_outline                                          = Engine::BuiltinShaders::Get( "Outline" );
	shader_texture_blit                                     = Engine::BuiltinShaders::Get( "Texture Blit" );

/* Instancing Data: */
	ResetInstanceData();

/* Initial transforms: */
	ground_transform
		.SetScaling( 30.0f, 60.0f, 1.0f )
		.SetRotation( 0.0_deg, 90.0_deg, 0.0_deg );

	wall_front_transform
		.SetScaling( 30.0f, 15.0f, 1.0f )
		.SetTranslation( 0.0f, 7.5, 30.0f );
	wall_left_transform
		.SetScaling( 60.0f, 15.0f, 1.0f )
		.SetRotation( -90.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( -15.0f, 7.5f, 0.0f );
	wall_right_transform
		.SetScaling( 60.0f, 15.0f, 1.0f )
		.SetRotation( +90.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( +15.0f, 7.5f, 0.0f );
	wall_back_transform
		.SetScaling( 30.0f, 15.0f, 1.0f )
		.SetRotation( 180.0_deg, 0.0_deg, 0.0_deg )
		.SetTranslation( 0.0f, 7.5f, -30.0f );

	cube_parallax_transform.SetTranslation( -1.0f, 0.5f, -2.0f );

	sphere_transform.SetTranslation( cube_parallax_transform.GetTranslation() );
	sphere_transform.OffsetTranslation( Vector3::Right() * 4.0f );

	/* Keep the first 10 the same as the ones from LOGL: */
	for( auto cube_index = 0; cube_index < CUBE_REFLECTED_COUNT; cube_index++ )
	{
		Degrees angle( 20.0f * cube_index );
		cube_reflected_transform_array[ cube_index ]
			.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
			.SetTranslation( CUBE_REFLECTED_POSITIONS[ cube_index ] + Vector3::Up() * 5.0f );
	}

	for( auto cube_index = 0; cube_index < CUBE_REFLECTED_COUNT; cube_index++ )
		cube_reflected_instance_data_array[ cube_index ] = cube_reflected_transform_array[ cube_index ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.

	/* The rest of the cubes: */
	{
		struct RandomAnglePair
		{
			Radians xz_angle;
			Radians inclination_angle;
		};

		std::vector< RandomAnglePair > random_angles( CUBE_COUNT );
		
		constexpr Radians inclination_limit = 15.0_deg;

		for( int i = 0; i < CUBE_COUNT; i++ )
		{
			random_angles[ i ].xz_angle          = Engine::Math::Random::Generate( 0.0_rad, Engine::Constants< Radians >::Two_Pi() );
			random_angles[ i ].inclination_angle = Engine::Math::Random::Generate( 0.0_rad, inclination_limit );
		}

		constexpr int thread_work_size = 1000 ; // Number of elements each thread should process

		// Ensure CUBE_COUNT is divisible by thread_work_size or adjust accordingly.
		constexpr int thread_group_count = ( CUBE_COUNT + thread_work_size - 1 ) / thread_work_size;

		constexpr std::array< int, thread_group_count > thread_group_indices = [ thread_group_count ]()
		{
			std::array< int, thread_group_count > indices;
			std::iota( indices.begin(), indices.end(), 0 );
			return indices;
		}();

		std::for_each( std::execution::par, thread_group_indices.cbegin(), thread_group_indices.cend(), [ & ]( auto&& thread_group_index )
		{
			// Calculate the range of indices for this group:
			const int start_index = thread_group_index * thread_work_size;
			const int end_index   = Engine::Math::Min( start_index + thread_work_size, CUBE_COUNT );

			for( auto cube_index = start_index; cube_index < end_index; cube_index++ )
			{
				const Radians random_xz_angle          = random_angles[ cube_index ].xz_angle;
				const Radians random_inclination_angle = random_angles[ cube_index ].inclination_angle;
				Degrees angle( 20.0f * cube_index + random_inclination_angle );
				cube_transform_array[ cube_index ]
					.SetScaling( 0.3f )
					.SetRotation( Quaternion( angle, Vector3{ 1.0f, 0.3f, 0.5f }.Normalized() ) )
					.SetTranslation( CUBES_ORIGIN + 
									 Vector3( Engine::Math::Cos( random_xz_angle ), 
											  Engine::Math::Sin( random_inclination_angle ),
											  Engine::Math::Sin( random_xz_angle ) )
									 * ( float )( ( cube_index % 90 ) + 10 ) );
			}
		} );

		/* Last cube is reserved: Put it on the ground to test shadows etc.
		 * Above loop processes the last cube as well, but it is simply overwritten here. */
		cube_transform_array[ CUBE_COUNT - 1 ]
			.SetScaling( 1.0f )
			.SetRotation( {} )
			.SetTranslation( 1.0f, 0.5f, -3.0f );
	}

	for( auto cube_index = 0; cube_index < CUBE_COUNT; cube_index++ )
		cube_instance_data_array[ cube_index ] = cube_transform_array[ cube_index ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.

	window_transform_array[ 0 ].SetTranslation( Vector3( -1.5f,	5.0f, -0.48f ) );
	window_transform_array[ 1 ].SetTranslation( Vector3(  1.5f,	5.0f,  0.51f ) );
	window_transform_array[ 2 ].SetTranslation( Vector3(  0.0f,	5.0f,  0.7f  ) );
	window_transform_array[ 3 ].SetTranslation( Vector3( -0.3f,	5.0f, -2.3f  ) );
	window_transform_array[ 4 ].SetTranslation( Vector3(  0.5f,	5.0f, -0.6f  ) );

/* Vertex/Index Data: */
	cube_mesh = Engine::Mesh( Engine::Primitive::Indexed::Cube::Positions,
							  "Cube",
							  Engine::Primitive::Indexed::Cube::Normals,
							  Engine::Primitive::Indexed::Cube::UVs,
							  Engine::Primitive::Indexed::Cube::Indices,
							  Engine::Primitive::Indexed::Cube::Tangents );

	cube_mesh_fullscreen = Engine::Mesh( Engine::Primitive::NonIndexed::Cube_FullScreen::Positions,
										 "Cube (Fullscreen)",
										 { /* No normals.	*/ },
										 { /* No uvs.		*/ },
										 { /* No indices.	*/ } );

	quad_mesh_uvs_only = Engine::Mesh( Engine::Primitive::Indexed::Quad::Positions,
									   "Quad (UVs Only)",
									   { /* No normals. */ },
									   Engine::Primitive::Indexed::Quad::UVs,
									   Engine::Primitive::Indexed::Quad::Indices );

	quad_mesh = Engine::Mesh( Engine::Primitive::Indexed::Quad::Positions,
							  "Quad",
							  Engine::Primitive::Indexed::Quad::Normals,
							  Engine::Primitive::Indexed::Quad::UVs,
							  Engine::Primitive::Indexed::Quad::Indices,
							  Engine::Primitive::Indexed::Quad::Tangents );

	quad_mesh_fullscreen = Engine::Mesh( Engine::Primitive::NonIndexed::Quad_FullScreen::Positions,
										 "Quad (FullScreen)",
										 { /* No normals. */ },
										 Engine::Primitive::NonIndexed::Quad_FullScreen::UVs,
										 { /* No indices. */ } );

	cube_mesh_instanced = Engine::Mesh( cube_mesh,
										{
											Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4, INSTANCED_ATTRIBUTE_START }	// Transform.
										},
										reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
										CUBE_COUNT,
										GL_STATIC_DRAW );

	cube_reflected_mesh_instanced = Engine::Mesh( cube_mesh,
												  {
													  Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4, INSTANCED_ATTRIBUTE_START }	// Transform.
												  },
												  reinterpret_cast< std::vector< float >& >( cube_reflected_instance_data_array ),
												  CUBE_REFLECTED_COUNT,
												  GL_STATIC_DRAW );

	cube_mesh_instanced_with_color = Engine::Mesh( cube_mesh,
												   {
													   Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4, INSTANCED_ATTRIBUTE_START }, // Transform.
													   Engine::VertexInstanceAttribute{ 1, GL_FLOAT_VEC4, INSTANCED_ATTRIBUTE_START + 4 }	// Color.
												   },
												   reinterpret_cast< std::vector< float >& >( light_source_instance_data_array ),
												   LIGHT_POINT_COUNT,
												   GL_DYNAMIC_DRAW );


	constexpr std::array< Vector3, 6 > quad_mesh_positions_ndc
	( {
		{ -0.425f, 0.35f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.35f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{  0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{ -0.425f, 0.80f, -0.01f }, // A little z offset backwards to make sure this is rendered in front & shows.
		{ -0.425f, 0.35f, -0.01f }  // A little z offset backwards to make sure this is rendered in front & shows.
	} );

	quad_mesh_mirror = Engine::Mesh( quad_mesh_positions_ndc,
									 "Quad (Rear-view mirror)",
									 { /* No normals. */ },
									 Engine::Primitive::NonIndexed::Quad_FullScreen::UVs,
									 { /* No indices. */ } );

	sphere_mesh = Engine::Mesh( Engine::Primitive::Indexed::UVSphereTemplate::Positions< 40 >(),
								"Sphere",
								Engine::Primitive::Indexed::UVSphereTemplate::Normals< 40 >(),
								Engine::Primitive::Indexed::UVSphereTemplate::UVs< 40 >(),
								Engine::Primitive::Indexed::UVSphereTemplate::Indices< 40 >(),
								Engine::Primitive::Indexed::UVSphereTemplate::Tangents< 40 >() );

/* Lighting: */
	ResetLightingData();

	renderer->AddDirectionalLight( &light_directional );

	for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
		renderer->AddPointLight( &light_point_array[ index ] );

	renderer->AddSpotLight( &light_spot );

/* Materials: */
	ResetMaterialData();

/* Renderer: */
	light_sources_renderable = Engine::Renderable( &cube_mesh_instanced_with_color, &light_source_material, 
												   nullptr /* => No Transform here, as we will provide the Transforms as instance data. */ );
	renderer->AddRenderable( &light_sources_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	cube_renderable = Engine::Renderable( &cube_mesh_instanced, &cube_material,
										  nullptr /* => No Transform here, as we will provide the Transforms as instance data. */,
										  true /* => has shadows. */,
										  true /* => casts shadows. */ );
	renderer->AddRenderable( &cube_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	cube_parallax_renderable = Engine::Renderable( &cube_mesh, &wall_material,
												   &cube_parallax_transform, 
												   true /* => has shadows. */,
												   true /* => casts shadows. */ );
	renderer->AddRenderable( &cube_parallax_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	cube_reflected_renderable = Engine::Renderable( &cube_reflected_mesh_instanced, &cube_reflected_material,
													nullptr /* => No Transform here, as we will provide the Transforms as instance data. */,
													true /* => has shadows. */,
													true /* => casts shadows. */ );
	renderer->AddRenderable( &cube_reflected_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	ground_renderable = Engine::Renderable( &quad_mesh, &ground_material, 
											&ground_transform, 
											true /* => has shadows. */,
											true /* => casts shadows. */ );
	renderer->AddRenderable( &ground_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	wall_front_renderable = Engine::Renderable( &quad_mesh, &wall_material, &wall_front_transform, true /* => has shadows. */, true /* => casts shadows. */ );
	wall_left_renderable  = Engine::Renderable( &quad_mesh, &wall_material, &wall_left_transform,  true /* => has shadows. */, true /* => casts shadows. */ );
	wall_right_renderable = Engine::Renderable( &quad_mesh, &wall_material, &wall_right_transform, true /* => has shadows. */, true /* => casts shadows. */ );
	wall_back_renderable  = Engine::Renderable( &quad_mesh, &wall_material, &wall_back_transform,  true /* => has shadows. */, true /* => casts shadows. */ );
	renderer->AddRenderable( &wall_front_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );
	renderer->AddRenderable( &wall_left_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );
	renderer->AddRenderable( &wall_right_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );
	renderer->AddRenderable( &wall_back_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	sphere_renderable = Engine::Renderable( &sphere_mesh, &sphere_material, &sphere_transform, false /* => does not have shadows. */, true /* => casts shadows. */ );
	renderer->AddRenderable( &sphere_renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );

	for( auto i = 0; i < WINDOW_COUNT; i++ )
	{
		window_renderable_array[ i ] = Engine::Renderable( &quad_mesh_uvs_only, &window_material, &window_transform_array[ i ] );
		renderer->AddRenderable( &window_renderable_array[ i ], Engine::Renderer::QUEUE_ID_TRANSPARENT );
	}

	skybox_renderable = Engine::Renderable( &cube_mesh_fullscreen, &skybox_material );
	renderer->AddRenderable( &skybox_renderable, Engine::Renderer::QUEUE_ID_SKYBOX );

	// TODO: Do not create an explicit (or rather, Application-visible) Renderable for skybox; Make it Renderer-internal.

	/* Disable some RenderPasses & Renderables on start-up to decrease clutter. */
	renderer->ToggleQueue( Engine::Renderer::QUEUE_ID_TRANSPARENT, false );

/* Camera: */
	ResetCamera();

	/* This is the earliest place we can MaximizeWindow() at,
	 * because the Renderer will populate its Intrinsic UBO info only upon AddRenderable( <Renderable with a Shader using said UBO> ). */

	/* No need to Initialize Framebuffer related stuff as maximizing the window will cause them to be (re)initialized in OnFramebufferResizeEvent(). */

	Platform::MaximizeWindow();

/* Models: */
	if( auto config_file = std::ifstream( "config.ini" ) )
	{
		std::string token;
		config_file >> token;
		if( token == "test_model_path" )
		{
			config_file >> token /* '=' */ >> token;
			if( not token.empty() && std::filesystem::exists( token ) )
				ReloadModel( test_model_info, token, "Test Model" );
		}

		config_file >> token;
		if( token == "meteorite_model_path" )
		{
			config_file >> token /* '=' */ >> token;
			if( not token.empty() && std::filesystem::exists( token ) )
				if( ReloadModel( meteorite_model_info, token, "Meteorite" ) )
					ReplaceMeteoriteAndCubeRenderables( true );
		}
	}
}

void BloomDemoApplication::Shutdown()
{
	if( auto config_file = std::ofstream( "config.ini" ) )
	{
		config_file << "test_model_path = "			<< test_model_info.file_path		<< "\n";
		config_file << "meteorite_model_path = "	<< meteorite_model_info.file_path	<< "\n";
	}
}

//void BloomDemoApplication::Run()
//{
//
//}

void BloomDemoApplication::Update()
{
	Application::Update();

	auto log_group( gl_logger.TemporaryLogGroup( "Bloom Demo Update()" ) );

	// TODO: Separate application logs from GL logs.

	current_time_as_angle = Radians( time_current );
	const Radians current_time_mod_two_pi( std::fmod( time_current, Engine::Constants< float >::Two_Pi() ) );

	/* Light sources' transform: */
	constexpr Radians angle_increment( Engine::Constants< Radians >::Two_Pi() / LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		/* Light source transform: */
		Vector3 point_light_position_world_space;

		if( light_point_array_is_animated )
		{
			const auto& old_rotation( light_point_transform_array[ i ].GetRotation() );
			Radians old_heading, old_pitch, bank;
			Engine::Math::QuaternionToEuler( old_rotation, old_heading, old_pitch, bank );

			Radians new_angle( Engine::Constants< Radians >::Pi_Over_Two() + current_time_mod_two_pi + ( float )angle_increment * ( float )i );
			const Radians new_angle_mod_two_pi( std::fmod( new_angle.Value(), Engine::Constants< float >::Two_Pi() ) );

			const bool heading_instead_of_pitching = new_angle_mod_two_pi <= Engine::Constants< Radians >::Pi();
			
			const auto point_light_rotation( Engine::Math::EulerToMatrix( /* Offset heading by 45 degrees to prevent light sources rotating directly in front of the camera. */
																		  Engine::Math::Lerp( Engine::Constants< Radians >::Pi_Over_Four(),
																							  new_angle_mod_two_pi + Engine::Constants< Radians >::Pi_Over_Four(),
																							  float( heading_instead_of_pitching ) ),
																		  float( !heading_instead_of_pitching ) * ( new_angle_mod_two_pi - 3.0f * Engine::Constants< Radians >::Pi_Over_Two() ),
																		  bank ) );

			point_light_position_world_space = ( ( ( heading_instead_of_pitching ? Vector4::Forward() : Vector4::Up() ) * light_point_orbit_radius ) *
												 ( point_light_rotation * Engine::Matrix::Translation( CAMERA_ROTATION_ORIGIN ) ) ).XYZ();
			point_light_position_world_space.SetY( point_light_position_world_space.Y() + 2.0f );

			light_point_transform_array[ i ].SetRotation( Engine::Math::MatrixToQuaternion( point_light_rotation ) );
		}

		light_point_transform_array[ i ].SetTranslation( point_light_position_world_space );

		light_source_instance_data_array[ i ].transform = light_point_transform_array[ i ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.
		light_source_instance_data_array[ i ].color     = Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
	}

	cube_mesh_instanced_with_color.UpdateInstanceData( light_source_instance_data_array.data() );

	/* Instanced cube's transform: */
	{
		Radians old_heading, old_pitch, old_bank;
		Engine::Math::QuaternionToEuler( cube_transform_array[ CUBE_COUNT - 1 ].GetRotation(), old_heading, old_pitch, old_bank );
		cube_transform_array[ CUBE_COUNT - 1 ].SetRotation( old_heading + angle_increment * time_delta, old_pitch, old_bank );
		cube_instance_data_array[ CUBE_COUNT - 1 ] = cube_transform_array[ CUBE_COUNT - 1 ].GetFinalMatrix().Transposed(); // Vertex attribute matrices' major can not be flipped in GLSL.
		
		cube_mesh_instanced.UpdateInstanceData_Partial( std::span( cube_instance_data_array.data(), 1 ), 0 );
	}

	/* Parallax cube's transform: */
	{
		Radians old_heading, old_pitch, old_bank;
		Engine::Math::QuaternionToEuler( cube_parallax_transform.GetRotation(), old_heading, old_pitch, old_bank );
		cube_parallax_transform.SetRotation( old_heading + angle_increment * time_delta, old_pitch, old_bank );
	}

	/* Sphere's transform; Same as the parallax cube: */
	sphere_transform.SetRotation( cube_parallax_transform.GetRotation() );

	/* Camera transform: */
	if( camera_animation_is_enabled )
	{
		/* Orbit motion: */

		Engine::Math::Vector< Radians, 3 > old_euler_angles;
		Engine::Math::QuaternionToEuler( camera_transform.GetRotation(), old_euler_angles );
		// Don't modify X & Z euler angles; Allow the user to modify them.
		camera_transform.SetRotation( Engine::Math::EulerToQuaternion( -current_time_as_angle * 0.33f, old_euler_angles.X(), old_euler_angles.Z() ) );

		auto new_pos = CAMERA_ROTATION_ORIGIN + -camera_transform.Forward() * camera_animation_orbit_radius;
		new_pos.SetY( camera_transform.GetTranslation().Y() ); // Don't modify Y position; Allow the user to modify it.
		camera_transform.SetTranslation( new_pos );
	}
	else
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
		camera_transform.OffsetTranslation( camera_transform.Right()   * -camera_move_speed * time_delta );
	if( Platform::IsKeyPressed( Platform::KeyCode::KEY_D ) )
		camera_transform.OffsetTranslation( camera_transform.Right()   * +camera_move_speed * time_delta );

	renderer->Update();
}

void BloomDemoApplication::Render()
{
	Engine::Application::Render();

	{
		renderer->UpdatePerPass( Engine::Renderer::PASS_ID_LIGHTING, camera );
	}

	// TODO: Outline pass.

	/* Post-processing pass: Blit off-screen framebuffers to quads on the default or the editor framebuffer to actually display them: */
	{
		/* This pass does not utilize camera view/projection => no Renderer::Update() necessary. */
	}

	renderer->Render();
}

void BloomDemoApplication::RenderImGui()
{
	/* Reminder: The rest of the rendering code (namely, ImGui) will be working in sRGB for the remainder of this frame,
	 * as the last step in the application's rendering was to enable sRGB encoding for the final framebuffer (default framebuffer or the editor FBO). */

	/* Need to switch to the default framebuffer, so ImGui can render onto it. */
	renderer->ResetToDefaultFramebuffer();

	SetImGuiViewportImageID( renderer->FinalFramebuffer().ColorAttachment().Id().Get() );
	Application::RenderImGui();

	if( show_imgui_demo_window )
		ImGui::ShowDemoWindow();

	const auto& style = ImGui::GetStyle();

	if( ImGui::Begin( ICON_FA_CUBES " Models", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		enum class ModelLoadActionResult
		{
			Loaded, Unloaded, None
		};

		auto DrawModelLine = [ & ]( ModelInfo& model_info, const char* model_name ) -> ModelLoadActionResult
		{
			ModelLoadActionResult action = ModelLoadActionResult::None;

			ImGui::SeparatorText( model_name );

			ImGui::PushID( model_name );

			if( not model_info.file_path.empty() )
			{
				static char buffer[ 260 ];
				strncpy_s( buffer, model_info.file_path.c_str(), model_info.file_path.size() );
				ImGui::TextDisabled( "Loaded Model Path", buffer, ( int )model_info.file_path.size(), ImGuiInputTextFlags_ReadOnly );
				if( ImGui::Checkbox( "Receives Shadows", &model_info.is_receiving_shadows ) )
				{
					for( auto& renderable : model_info.model_instance.Renderables() )
						renderer->RemoveRenderable( &renderable );

					model_info.model_instance.ToggleShadowReceivingStatus( model_info.is_receiving_shadows );

					for( auto& renderable : model_info.model_instance.Renderables() )
						renderer->AddRenderable( &renderable, Engine::Renderer::QUEUE_ID_GEOMETRY );
				}
				ImGui::SameLine();
				if( ImGui::Checkbox( "Casts Shadows", &model_info.is_casting_shadows ) )
					model_info.model_instance.ToggleShadowCastingStatus( model_info.is_casting_shadows );

				if( ImGui::Button( ICON_FA_FOLDER_OPEN " Reload" ) )
				{
					if( auto maybe_file_name = Platform::BrowseFileName( { "glTF (*.gltf;*.glb)",		"*.gltf;*.glb",
																			"Standard glTF (*.gltf)",	"*.gltf",
																			"Binary glTF (*.glb)",		"*.glb" },
																		 "Choose a Model to Load" );
						maybe_file_name.has_value() && *maybe_file_name != model_info.file_path )
					{
						if( ReloadModel( model_info, *maybe_file_name, model_name ) )
							action = ModelLoadActionResult::Loaded;
					}
				}
				ImGui::SameLine();
				if( ImGui::Button( ICON_FA_XMARK " Unload" ) )
				{
					UnloadModel( model_info );
					action = ModelLoadActionResult::Unloaded;
				}
			}
			else
			{
				const auto button_size( ImGui::CalcTextSize( ICON_FA_CUBES " Models   " ) + style.ItemInnerSpacing );
				if( ImGui::Button( ICON_FA_FOLDER_OPEN " Load", button_size ) )
				{
					if( auto maybe_file_name = Platform::BrowseFileName( { "glTF (*.gltf;*.glb)",		"*.gltf;*.glb",
																			"Standard glTF (*.gltf)",	"*.gltf",
																			"Binary glTF (*.glb)",		"*.glb" },
																		 "Choose a Model to Load" );
						maybe_file_name.has_value() && *maybe_file_name != model_info.file_path )
					{
						if( ReloadModel( model_info, *maybe_file_name, model_name ) )
							action = ModelLoadActionResult::Loaded;
					}
				}
			}

			ImGui::PopID();

			return action;
		};

		DrawModelLine( test_model_info, "Test Model" );
		switch( DrawModelLine( meteorite_model_info, "Meteorite" ) )
		{
			case ModelLoadActionResult::Loaded:
				ReplaceMeteoriteAndCubeRenderables( true );
				break;
			case ModelLoadActionResult::Unloaded:
				ReplaceMeteoriteAndCubeRenderables( false );
				break;
			default:
				break;
		}
	}

	ImGui::End();

	Engine::ImGuiDrawer::Draw( skybox_material, *renderer );
	Engine::ImGuiDrawer::Draw( light_source_material, *renderer );
	Engine::ImGuiDrawer::Draw( ground_material, *renderer );
	Engine::ImGuiDrawer::Draw( wall_material, *renderer );
	Engine::ImGuiDrawer::Draw( sphere_material, *renderer );
	Engine::ImGuiDrawer::Draw( window_material, *renderer );
	Engine::ImGuiDrawer::Draw( cube_material, *renderer );
	Engine::ImGuiDrawer::Draw( cube_reflected_material, *renderer );
	for( auto& test_material : test_model_info.model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ), *renderer );
	for( auto& test_material : meteorite_model_info.model_instance.Materials() )
		Engine::ImGuiDrawer::Draw( const_cast< Engine::Material& >( test_material ), *renderer );
	Engine::ImGuiDrawer::Draw( outline_material, *renderer );
	Engine::ImGuiDrawer::Draw( mirror_quad_material, *renderer );

	if( ImGui::Begin( "Instance Data" ) )
	{
		ImGui::SeparatorText( "Light Sources" );

		if( ImGui::BeginTable( "Light Sources", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
		{
			ImGui::TableSetupColumn( "#" );
			ImGui::TableSetupColumn( "Color" );
			ImGui::TableSetupColumn( "Transform" );

			ImGui::TableHeadersRow();
			ImGui::TableNextRow();

			const auto first_column_width = ImGui::CalcTextSize( " 999" ).x + style.ItemInnerSpacing.x * 2;

			for( auto index = 0; index < LIGHT_POINT_COUNT; index++ )
			{
				const auto& instance_data = light_source_instance_data_array[ index ];

				ImGui::PushID( index );

				ImGui::TableNextColumn();
				int no = index + 1;
				ImGui::PushItemWidth( first_column_width );
				ImGui::InputInt( "", &no, 0, 0, ImGuiInputTextFlags_ReadOnly );
				ImGui::PopItemWidth();

				ImGui::TableNextColumn();
				Engine::ImGuiDrawer::Draw( instance_data.color );
				
				ImGui::TableNextColumn();
				Engine::ImGuiDrawer::Draw( instance_data.transform );


				ImGui::PopID();
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();

	if( ImGui::Begin( ICON_FA_LIGHTBULB " Lighting", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
	{
		if( ImGui::Checkbox( "Enabled##AllLights", &light_is_enabled ) )
		{
			light_directional.is_enabled = light_is_enabled;

			for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
			{
				light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
				light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
			}

			light_spot.is_enabled = light_is_enabled;
		}

		ImGui::SameLine( 0.0f, 20.0f );

		if( ImGui::Button( ICON_FA_ARROWS_ROTATE " Reset##Lights" ) )
			ResetLightingData();

		ImGui::NewLine();

		if( ImGui::BeginTabBar( "Lights Tab Bar", ImGuiTabBarFlags_DrawSelectedOverline | ImGuiTabBarFlags_NoTabListScrollingButtons ) )
		{
			if( ImGui::BeginTabItem( "Directional Light" ) )
			{
				Engine::ImGuiDrawer::Draw( light_directional, "Directional Light" );
				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Point Lights" ) )
			{
				Engine::ImGuiUtility::BeginGroupPanel( "Options" );
				if( ImGui::Checkbox( "Disable All", &light_point_array_disable ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						light_point_array[ i ].is_enabled = light_is_enabled && not light_point_array_disable;
						light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
					}
				}
				ImGui::Checkbox( "Animate (Orbit) Point Lights", &light_point_array_is_animated );
				if( light_point_array_is_animated )
					ImGui::SliderFloat( "Light Orbit Radius", &light_point_orbit_radius, 0.0f, 30.0f );
				Engine::ImGuiUtility::EndGroupPanel();

				if( ImGui::TreeNodeEx( "Point Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
					{
						const std::string name( "Point Light # " + std::to_string( i ) );
						const bool was_enabled = light_point_array[ i ].is_enabled;
						if( Engine::ImGuiDrawer::Draw( light_point_array[ i ], name.c_str(), light_point_array_is_animated /* hide position. */ ) )
						{
							light_source_instance_data_array[ i ].color = Engine::Color4( light_point_array[ i ].data.diffuse_and_attenuation_linear.color, 1.0f );
							if( was_enabled != light_point_array[ i ].is_enabled )
								light_source_instance_data_array[ i ].color.SetW( 1.0f - light_source_instance_data_array[ i ].color.W() );
						}
					}

					ImGui::TreePop();
				}

				ImGui::EndTabItem();
			}

			if( ImGui::BeginTabItem( "Spot Lights" ) )
			{
				if( ImGui::TreeNodeEx( "Spot Lights", ImGuiTreeNodeFlags_Framed ) )
				{
					Engine::ImGuiDrawer::Draw( light_spot, "Spot Light" );

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

		ImGui::Checkbox( "Animate (Rotate) Camera", &camera_animation_is_enabled );
		if( camera_animation_is_enabled )
			ImGui::SliderFloat( "Camera Orbit Radius", &camera_animation_orbit_radius, 0.0f, 50.0f );
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

	renderer->RenderImGui();
}

void BloomDemoApplication::OnKeyboardEvent( const Platform::KeyCode key_code, const Platform::KeyAction key_action, const Platform::KeyMods key_mods )
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
		case Platform::KeyCode::KEY_U:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Min( light_spot.data.cutoff_angle_inner + 0.33_deg, light_spot.data.cutoff_angle_outer );
			break;
		case Platform::KeyCode::KEY_Y:
			if( key_action == Platform::KeyAction::PRESS || key_action == Platform::KeyAction::REPEAT )
				light_spot.data.cutoff_angle_inner = Engine::Math::Max( light_spot.data.cutoff_angle_inner - 0.33_deg, 0_deg );
			break;
		case Platform::KeyCode::KEY_I:
			if( key_action == Platform::KeyAction::PRESS )
			{
				show_imgui = !show_imgui;
				if( show_imgui )
				{
					renderer->SetFinalPassToUseFinalFramebuffer();
				}
				else
				{
					renderer->SetFinalPassToUseDefaultFramebuffer();
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

void BloomDemoApplication::OnFramebufferResizeEvent( const int width_new_pixels, const int height_new_pixels )
{
	/* Do nothing on minimize: */
	if( width_new_pixels == 0 || height_new_pixels == 0 || 
		( renderer->FinalFramebuffer().Size() == Vector2I{ width_new_pixels, height_new_pixels } ) )
		return;

	renderer->OnFramebufferResize( width_new_pixels, height_new_pixels );

	RecalculateProjectionParameters( width_new_pixels, height_new_pixels );
	
	// TODO: Move these into Renderer: Maybe Materials can have a sort of requirements info. (or dependencies) and the Renderer can automatically update Material info such as the ones below.

	mirror_quad_material.SetTexture( "uniform_tex", &renderer->CustomFramebuffer( 0 ).ColorAttachment() );
}

void BloomDemoApplication::OnFramebufferResizeEvent( const Vector2I new_size_pixels )
{
	OnFramebufferResizeEvent( new_size_pixels.X(), new_size_pixels.Y() );
}

void BloomDemoApplication::ResetLightingData()
{
	light_directional_transform = Engine::Transform();

	light_directional = 
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{ 0.1f, 0.1f, 0.1f },
			.diffuse  = Engine::Color3{ 0.4f, 0.4f, 0.4f },
			.specular = Engine::Color3{ 0.5f, 0.5f, 0.5f },
		},
		.transform = &light_directional_transform
	};

	light_is_enabled = true;

	light_point_array_disable      = false;
	light_point_array_is_animated  = true;
	light_point_orbit_radius       = 20.0f;

	light_spot_array_disable = false;

	light_point_array.resize( LIGHT_POINT_COUNT );
	for( auto i = 0; i < LIGHT_POINT_COUNT; i++ )
	{
		const auto random_color( Engine::Math::Random::Generate< Engine::Color3 >() );
		light_point_array[ i ] =
		{
			.is_enabled = true,
			.data =
			{
				.ambient_and_attenuation_constant = { .color = {},			 .scalar = 0.06f	},
				.diffuse_and_attenuation_linear   = { .color = random_color, .scalar = 0.001f	},
				.specular_attenuation_quadratic   = { .color = random_color, .scalar = 0.0375f	},
			},
			.transform = &light_point_transform_array[ i ]
		};
	}

	light_spot_transform = Engine::Transform();

	light_spot =
	{
		.is_enabled = true,
		.data =
		{
			.ambient  = Engine::Color3{  0.05f,  0.05f,  0.03f },
			.diffuse  = Engine::Color3{  0.4f,   0.4f,   0.27f  },
			.specular = Engine::Color3{  0.5f,   0.5f,   0.33f  },

		/* End of GLSL equivalence. */
			.cutoff_angle_inner = 12.5_deg,
			.cutoff_angle_outer = 17.5_deg
		},
		.transform = &light_spot_transform
	};
}

void BloomDemoApplication::ResetMaterialData()
{
	skybox_material = Engine::Material( "Skybox", shader_skybox );
	skybox_material.SetTexture( "uniform_tex", skybox_texture );

	light_source_material = Engine::Material( "Light Source", shader_basic_color_instanced );
	
	/* Set the first cube's material to Blinn-Phong shader w/ skybox reflection: */
	cube_reflected_material = Engine::Material( "Cube (Reflected)", shader_blinn_phong_skybox_reflection_shadowed_instanced );
	cube_reflected_material.SetTexture( "uniform_tex_diffuse", Engine::BuiltinTextures::Get( "UV Test" ) );
	cube_reflected_material.SetTexture( "uniform_tex_specular", Engine::BuiltinTextures::Get( "Black" ) );
	cube_reflected_material.SetTexture( "uniform_tex_normal", container_texture_normal_map );
	cube_reflected_material.SetTexture( "uniform_tex_reflection", Engine::BuiltinTextures::Get( "Black" ) );
	cube_reflected_material.SetTexture( "uniform_tex_skybox", skybox_texture );
	cube_reflected_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	cube_reflected_material.Set( "uniform_reflectivity", 1.0f );

	cube_material = Engine::Material( "Cube", shader_blinn_phong_shadowed_instanced );
	cube_material.SetTexture( "uniform_tex_diffuse", container_texture_diffuse_map );
	cube_material.SetTexture( "uniform_tex_specular", container_texture_specular_map );
	cube_material.SetTexture( "uniform_tex_normal", container_texture_normal_map );
	cube_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	ground_material = Engine::Material( "Ground", shader_blinn_phong_shadowed );
	ground_material.SetTexture( "uniform_tex_diffuse", checker_pattern_texture );
	ground_material.SetTexture( "uniform_tex_specular", checker_pattern_texture );
	const auto& ground_quad_scale( ground_transform.GetScaling().XY() );
	Vector4 ground_texture_scale_and_offset( ground_quad_scale.X(), ground_quad_scale.Y() /* Offset is 0 so no need to set it explicitly. */ );
	ground_material.Set( "uniform_texture_scale_and_offset", ground_texture_scale_and_offset );

	wall_material = Engine::Material( "Wall", shader_blinn_phong_shadowed_parallax );
	wall_material.SetTexture( "uniform_tex_diffuse", brickwall_diffuse_map );
	wall_material.SetTexture( "uniform_tex_specular", checker_pattern_texture );
	wall_material.SetTexture( "uniform_tex_normal", brickwall_normal_map );
	wall_material.SetTexture( "uniform_tex_parallax_height", brickwall_displacement_map );
	const auto& front_wall_quad_scale( wall_front_transform.GetScaling().XY() / 10.0f );
	Vector4 front_wall_texture_scale_and_offset( front_wall_quad_scale /* Offset is 0 so no need to set it explicitly. */ );
	wall_material.Set( "uniform_texture_scale_and_offset", front_wall_texture_scale_and_offset );
	wall_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	wall_material.Set( "uniform_parallax_height_scale", 0.1f );
	wall_material.Set( "uniform_parallax_depth_layer_count_min_max", Vector2U( 8u, 32u ) );

	sphere_material = Engine::Material( "Sphere", shader_blinn_phong_skybox_reflection );
	sphere_material.SetTexture( "uniform_tex_diffuse", Engine::ServiceLocator< Engine::BuiltinTextures >::Get().Get( "UV Test" ) );
	//sphere_material.SetTexture( "uniform_tex_reflection", container_texture_specular_map );
	sphere_material.SetTexture( "uniform_tex_skybox", skybox_texture );
	sphere_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );
	sphere_material.Set( "uniform_reflectivity", 0.9f );

	window_material = Engine::Material( "Transparent Window", shader_basic_textured );
	window_material.SetTexture( "uniform_tex", transparent_window_texture );
	window_material.Set( "uniform_texture_scale_and_offset", Vector4( 1.0f, 1.0f, 0.0f, 0.0f ) );

	outline_material = Engine::Material( "Outline", shader_outline );

	mirror_quad_material = Engine::Material( "Rear-view Mirror", shader_texture_blit );

	ground_quad_surface_data = wall_surface_data = cube_surface_data =
	{
		.color_diffuse       = {},
		.has_texture_diffuse = 1,
		.shininess           = 32.0f
	};

	ground_material.Set( "BlinnPhongMaterialData", ground_quad_surface_data );
	wall_material.Set( "BlinnPhongMaterialData", wall_surface_data );
	cube_material.Set( "BlinnPhongMaterialData", cube_surface_data );
	cube_reflected_material.Set( "BlinnPhongMaterialData", cube_surface_data );
	sphere_material.Set( "BlinnPhongMaterialData", cube_surface_data );

	outline_material.Set( "uniform_color", Engine::Color4::Orange() );
	outline_material.Set( "uniform_outline_thickness", 0.1f );
}

void BloomDemoApplication::ResetInstanceData()
{
	cube_instance_data_array.resize( CUBE_COUNT );
	cube_reflected_instance_data_array.resize( CUBE_REFLECTED_COUNT );
	light_source_instance_data_array.resize( LIGHT_POINT_COUNT );
}

void BloomDemoApplication::ResetCamera()
{
	camera_animation_orbit_radius = 30.0f;

	ResetProjection();

	SwitchCameraView( CameraView::FRONT );
}

void BloomDemoApplication::ResetProjection()
{
	camera = Engine::Camera( &camera_transform, camera.GetAspectRatio(), camera.GetVerticalFieldOfView() ); // Keep current aspect ratio & v-fov.
}

void BloomDemoApplication::SwitchCameraView( const CameraView view )
{
	camera_animation_is_enabled = false;

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
			camera_transform.SetTranslation( 12.0f, 10.0f, -14.0f );
			camera_transform.SetRotation( -35_deg, 0_deg, 0_deg );
			break;

		default:
			break;
	}

	camera_controller.ResetToTransform();
}

BloomDemoApplication::Radians BloomDemoApplication::CalculateVerticalFieldOfView( const Radians horizontal_field_of_view, const float aspect_ratio ) const
{
	return 2.0f * Engine::Math::Atan2( Engine::Math::Tan( horizontal_field_of_view / 2.0f ), aspect_ratio );
}

bool BloomDemoApplication::ReloadModel( ModelInfo& model_info_to_be_loaded, const std::string& file_path, const char* name )
{
	if( auto new_model = Engine::AssetDatabase< Engine::Model >::CreateAssetFromFile( name, file_path ); 
		new_model )
	{
		model_info_to_be_loaded.file_path = file_path;

		auto& model_instance_to_load_into = model_info_to_be_loaded.model_instance;

		for( auto& renderable_to_remove : model_instance_to_load_into.Renderables() )
			renderer->RemoveRenderable( &renderable_to_remove );

		model_instance_to_load_into = Engine::ModelInstance( new_model,
															 model_info_to_be_loaded.shader,
															 model_info_to_be_loaded.shader_shadow_receiving,
															 Vector3::One(),
															 Quaternion(),
															 Vector3::Up() * 8.0f,
															 nullptr,
															 model_info_to_be_loaded.is_receiving_shadows,
															 model_info_to_be_loaded.is_casting_shadows,
															 Vector4{ 1.0f, 1.0f, 0.0f, 0.0f } );

		for( auto& renderable_to_add : model_instance_to_load_into.Renderables() )
			renderer->AddRenderable( &renderable_to_add, Engine::Renderer::QUEUE_ID_GEOMETRY );

		return true;
	}

	return false;
}

void BloomDemoApplication::UnloadModel( ModelInfo& model_info_to_be_loaded )
{
	model_info_to_be_loaded.file_path = "";

	for( auto& renderable_to_remove : model_info_to_be_loaded.model_instance.Renderables() )
		renderer->RemoveRenderable( &renderable_to_remove );

	model_info_to_be_loaded.model_instance = {};
}

void BloomDemoApplication::ReplaceMeteoriteAndCubeRenderables( bool use_meteorites )
{
	if( use_meteorites )
	{
		meteorite_renderable = &meteorite_model_info.model_instance.Renderables().front();
		cube_mesh_instanced = Engine::Mesh( *meteorite_renderable->GetMesh(),
											{
												Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
											},
											reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
											CUBE_COUNT,
											GL_STATIC_DRAW );
		meteorite_renderable->SetMesh( &cube_mesh_instanced );
		renderer->RemoveRenderable( &cube_renderable );
	}
	else
	{
		renderer->RemoveRenderable( meteorite_renderable );
		meteorite_renderable = nullptr;
		cube_mesh_instanced = Engine::Mesh( cube_mesh,
											{
												Engine::VertexInstanceAttribute{ 1, GL_FLOAT_MAT4 }	// Transform.
											},
											reinterpret_cast< std::vector< float >& >( cube_instance_data_array ),
											CUBE_COUNT,
											GL_STATIC_DRAW );
		renderer->AddRenderable( &cube_renderable );
	}
}

void BloomDemoApplication::RecalculateProjectionParameters( const int width_new_pixels, const int height_new_pixels )
{
	camera.SetAspectRatio( float( width_new_pixels ) / height_new_pixels );
	camera.SetVerticalFieldOfView( CalculateVerticalFieldOfView( Engine::Constants< Radians >::Pi_Over_Two(), camera.GetAspectRatio() ) );
}

void BloomDemoApplication::RecalculateProjectionParameters( const Vector2I new_size_pixels )
{
	RecalculateProjectionParameters( new_size_pixels.X(), new_size_pixels.Y() );
}

void BloomDemoApplication::RecalculateProjectionParameters()
{
	RecalculateProjectionParameters( renderer->FinalFramebuffer().Size() );
}
