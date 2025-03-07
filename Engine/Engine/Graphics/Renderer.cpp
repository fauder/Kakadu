// Engine Includes.
#include "Renderer.h"
#include "DefaultFramebuffer.h"
#include "BuiltinShaders.h"
#include "BuiltinTextures.h"
#include "Core/ImGuiCustomColors.h"
#include "Core/ImGuiDrawer.hpp"
#include "Core/ImGuiUtility.h"
#include "Primitive/Primitive_Quad_FullScreen.h"

// Vendor Includes.
#include <IconFontCppHeaders/IconsFontAwesome6.h>

#ifdef _EDITOR
#define CONSOLE_ERROR( message ) logger.Error( ICON_FA_DRAW_POLYGON " " message )
#else
#define CONSOLE_ERROR( message ) do {} while( false )
#endif // _EDITOR

#ifdef _EDITOR
#define CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( function_name, pass_id )\
if( not render_pass_map.contains( pass_id ) )\
{\
	logger.Error( ICON_FA_DRAW_POLYGON " " function_name "() called for non-existing pass." );\
	return;\
}
#else
#define CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( function_name, pass_id ) do {} while( false )
#endif // _EDITOR

#ifdef _EDITOR
#define CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( function_name, queue_id )\
if( not render_queue_map.contains( queue_id ) )\
{\
	logger.Error( ICON_FA_DRAW_POLYGON " " function_name "() called for non-existing queue." );\
	return;\
}
#else
#define CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( function_name, queue_id ) do {} while( false )
#endif // _EDITOR

namespace Engine
{
	Renderer::Renderer( const bool enable_gamma_correction,
						Texture::Format main_framebuffer_color_format,
						std::uint8_t main_framebuffer_msaa_sample_count,
						const std::initializer_list< Framebuffer::Description > custom_framebuffer_descriptions )
		:
		logger( ServiceLocator< GLLogger >::Get() ),
		framebuffer_current( nullptr ),
		framebuffer_main_msaa_sample_count( main_framebuffer_msaa_sample_count ),
		framebuffer_main_color_format( main_framebuffer_color_format ),
		lights_point_active_count( 0 ),
		lights_spot_active_count( 0 ),
		shaders_need_uniform_buffer_lighting( false ),
		shaders_need_uniform_buffer_other( false ),
		framebuffer_sRGB_encoding_is_enabled( false ),
		gamma_correction_is_enabled( enable_gamma_correction )
	{
		logger.IgnoreID( 131185 ); // "Buffer object will use VIDEO mem..." log.

		if( custom_framebuffer_descriptions.size() <= FRAMEBUFFER_CUSTOM_AVAILABLE_COUNT )
			std::copy( custom_framebuffer_descriptions.begin(), custom_framebuffer_descriptions.end(), framebuffer_custom_description_array.begin() );
		else
			CONSOLE_ERROR( "Renderer: Number of custom framebuffers requested exceeds the max. available limit." );

		Texture::ToggleGammaCorrection( gamma_correction_is_enabled );

		DefaultFramebuffer::Instance(); // Initialize.
		
		BuiltinShaders::Initialize( *this );
		BuiltinTextures::Initialize();

		if( shaders_need_uniform_buffer_lighting )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_SHADOW_BIAS_MIN_MAX_2_RESERVED", Vector4( 0.005f, 0.05f, 0.0f, 0.0f ) );
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_SHADOW_SAMPLE_COUNT_X_Y",		Vector2I( 3, 3 ) );
		}

		InitializeBuiltinQueues();
		InitializeBuiltinPasses();

		InitializeInternalMeshes();
		InitializeInternalShaders();
		InitializeInternalMaterials();
		InitializeInternalRenderables();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update()
	{
		RecompileModifiedShaders();

		CalculateShadowMappingInformation();
	}

	void Renderer::UpdatePerPass( const RenderPass::ID pass_id_to_update, Camera& camera )
	{
#ifdef _EDITOR
		if( not render_pass_map.contains( pass_id_to_update ) )
			CONSOLE_ERROR( " UpdatePerPass() called for non-existing pass." );
#endif // _EDITOR

		auto& pass = render_pass_map[ pass_id_to_update ];

		pass.view_matrix       = camera.GetViewMatrix();
		pass.projection_matrix = camera.GetProjectionMatrix();

		if( camera.UsesPerspectiveProjection() )
		{
			pass.plane_near             = camera.GetNearPlaneOffset();
			pass.plane_far              = camera.GetFarPlaneOffset();
			pass.aspect_ratio           = camera.GetAspectRatio();
			pass.vertical_field_of_view = camera.GetVerticalFieldOfView();
		}
	}

	void Renderer::Render()
	{
		for( auto& [ pass_id, pass ] : render_pass_map )
		{
			if( PassHasContentToRender( pass ) )
			{
				const auto log_group( logger.TemporaryLogGroup( ( "[Pass]:" + pass.name ).c_str() ) );

				SetIntrinsicsPerPass( pass );

				UploadIntrinsics();
				UploadGlobals();

				SetRenderState( pass.render_state, pass.target_framebuffer, pass.clear_framebuffer );

				for( auto& queue_id : pass.queue_id_set )
				{
					if( auto& queue = render_queue_map[ queue_id ]; 
						QueueHasContentToRender( queue ) )
					{
						const auto log_group( logger.TemporaryLogGroup( ( "[Queue]:" + queue.name ).c_str() ) );

						// TODO: Do not set render state for state that is not changing (i.e., dirty check).
						if( pass.render_state_override_is_allowed )
							SetRenderState( queue.render_state_override, pass.target_framebuffer /* No clearing for queues. */ );

						SortRenderablesInQueue( current_camera_info.view_matrix.GetRow< 3 >( 3 ), queue.renderable_list, queue.render_state_override.sorting_mode );

						switch( pass_id )
						{
							case PASS_ID_SHADOW_MAPPING:
							{
								static auto& shadow_map_write_shader           = *BuiltinShaders::Get( "Shadow-map Write" );
								static auto& shadow_map_write_instanced_shader = *BuiltinShaders::Get( "Shadow-map Write (Instanced)" );
								
								shadow_map_write_shader.Bind();

								for( auto& renderable : queue.renderable_list )
								{
									if( renderable->is_enabled && renderable->is_receiving_shadows && not renderable->mesh->HasInstancing() )
									{
										renderable->mesh->Bind();

										if( renderable->transform )
											shadow_map_write_shader.SetUniform( "uniform_transform_world", renderable->transform->GetFinalMatrix() );

										Render( *renderable->mesh );
									}
								}

								shadow_map_write_instanced_shader.Bind();

								for( auto& renderable : queue.renderable_list )
								{
									if( renderable->is_enabled && renderable->is_receiving_shadows && renderable->mesh->HasInstancing() )
									{
										renderable->mesh->Bind();

										Render( *renderable->mesh );
									}
								}
							}
							break;

							default: // "Regular" passes:
							{
								for( const auto& [ shader_name, shader ] : queue.shaders_in_flight )
								{
									shader->Bind();

									for( auto& [ material_name, material ] : queue.materials_in_flight )
									{
										if( material->shader->Id() == shader->Id() )
										{
											material->UploadUniforms();

											for( auto& renderable : queue.renderable_list )
											{
												if( renderable->is_enabled && renderable->material->Name() == material_name )
												{
													renderable->mesh->Bind();

													if( renderable->transform )
														material->SetAndUploadUniform( "uniform_transform_world", renderable->transform->GetFinalMatrix() );

													Render( *renderable->mesh );
												}
											}
										}
									}
								}
							}
							break;
						}
					}
				}
			}
		}
	}

	void Renderer::RenderImGui()
	{
		if( ImGui::Begin( ICON_FA_DRAW_POLYGON " Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			ImGui::SeparatorText( "General" );

			ImGuiUtility::ImmutableCheckbox( "Gamma Correction", gamma_correction_is_enabled );

			ImGui::SeparatorText( "Passes & Queues" );

			if( ImGui::BeginTable( "Passes & Queues", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
			{
				ImGui::TableSetupColumn( "Rendering" );
				ImGui::TableSetupColumn( "Pass" );
				ImGui::TableSetupColumn( "Target" );

				ImGui::TableNextRow( ImGuiTableRowFlags_Headers ); // Indicates that the header row will be modified
				ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 0,
																			"A pass will not render if:\n"
																			"\t" ICON_FA_ARROW_RIGHT " All its queues are empty/disabled, \n"
																			"\t" ICON_FA_ARROW_RIGHT " AND/OR All renderables inside those queues are all disabled." );
				ImGuiUtility::Table_Header_ManuallySubmit( std::array< int, 2 >{ 1, 2 } );
				ImGui::TableNextRow();

				for( auto& [ pass_id, pass ] : render_pass_map )
				{
					ImGui::TableNextColumn();
					ImGui::PushID( ( int )pass_id );

					const bool pass_has_content_to_render = PassHasContentToRender( pass );

					if( not pass_has_content_to_render )
						ImGuiUtility::BeginDisabledButInteractable();

					ImGuiUtility::CenterCheckbox();
					ImGuiUtility::ImmutableCheckbox( "##p_active_checkb", pass_has_content_to_render );

					ImGui::TableNextColumn();

					ImGui::Checkbox( "", &pass.is_enabled );

					ImGui::PopID();
					ImGui::SameLine();
					if( ImGui::TreeNodeEx( pass.name.c_str(), 0, "Pass [%d]: %s", ( int )pass_id, pass.name.c_str()) )
					{
						// TODO: Display RenderState info as a collapseable header.
						for( auto& queue_id : pass.queue_id_set )
						{
							auto& queue = render_queue_map[ queue_id ];

							if( queue.renderable_list.empty() )
							{
								ImGui::TextDisabled( "Empty Queue [%d]: %s", ( int )queue_id, queue.name.c_str() );
								continue;
							}

							const bool queue_has_content_to_render = QueueHasContentToRender( queue );

							if( not queue_has_content_to_render )
								ImGuiUtility::BeginDisabledButInteractable();

							ImGui::PushID( ( int )queue_id );
							ImGui::Checkbox( "", &queue.is_enabled );
							ImGui::PopID();
							ImGui::SameLine();
							if( ImGui::TreeNodeEx( queue.name.c_str(), 0, "Queue [%d]: %s", ( int )queue_id, queue.name.c_str() ) )
							{
								ImGui::BeginDisabled( not queue.is_enabled );

								for( const auto& [ shader_name, shader ] : queue.shaders_in_flight )
								{
									for( const auto& [ material_name, material ] : queue.materials_in_flight )
									{
										if( material->shader->Id() == shader->Id() )
										{
											for( auto& renderable : queue.renderable_list )
											{
												if( renderable->material->Name() == material_name )
												{
													ImGui::PushID( renderable );
													ImGui::Checkbox( "", &renderable->is_enabled );
													ImGui::PopID();
													ImGui::BeginDisabled( not renderable->is_enabled );
													ImGui::SameLine(); ImGui::TextUnformatted( renderable->material->name.c_str() );
													if( renderable->mesh->HasInstancing() )
													{
														int instance_Count = renderable->mesh->InstanceCount();
														ImGui::SameLine(); ImGui::TextColored( ImVec4( 0.84f, 0.59f, 0.45f, 1.0f ), "(Instance Count: %d)", instance_Count );
													}
													ImGui::EndDisabled();
												}
											}
										}
									}
								}

								ImGui::EndDisabled();

								ImGui::TreePop();
							}

							if( not queue_has_content_to_render )
								ImGuiUtility::EndDisabledButInteractable();
						}

						ImGui::TreePop();
					}

					ImGui::TableNextColumn();
					ImGui::TextUnformatted( pass.target_framebuffer->Name().c_str() );

					if( not pass_has_content_to_render )
						ImGuiUtility::EndDisabledButInteractable();
				}

				ImGui::EndTable();
			}

			/* Framebuffers: */
			ImGui::SeparatorText( "Framebuffers" );

			auto DrawFramebufferImGui_Decorations = []( const Framebuffer& framebuffer )
			{
				if( framebuffer.IsMultiSampled() )
				{
					ImGui::SameLine();
					ImGuiUtility::DrawRoundedRectText( "MSAA", ImGuiCustomColors::COLOR_MSAA );
				}

				if( framebuffer.IsHDR() )
				{
					ImGui::SameLine();
					ImGuiUtility::DrawRoundedRectText( "HDR", ImGuiCustomColors::COLOR_HDR );
				}

				if( framebuffer.Is_sRGB() )
				{
					ImGui::SameLine();
					ImGuiUtility::DrawRainbowRectText( "sRGB" );
				}
			};

			auto DrawFramebufferImGui = [ & ]( const Framebuffer& framebuffer )
			{
				if( framebuffer.IsValid() )
				{
					if( ImGui::TreeNodeEx( framebuffer.name.c_str() ) )
					{
						DrawFramebufferImGui_Decorations( framebuffer );

						ImGuiDrawer::Draw( framebuffer );
						ImGui::TreePop();
					}
					else
						DrawFramebufferImGui_Decorations( framebuffer );
				}
			};

			auto DrawCustomFramebufferImGui = [ & ]( const Framebuffer& custom_framebuffer )
			{
				const auto name = custom_framebuffer.name.c_str();
				if( custom_framebuffer.IsValid() )
				{
					if( ImGui::TreeNodeEx( name, ImGuiTreeNodeFlags_None, "[Custom] %s", name ) )
					{
						DrawFramebufferImGui_Decorations( custom_framebuffer );

						ImGuiDrawer::Draw( custom_framebuffer );
						ImGui::TreePop();
					}
					else
						DrawFramebufferImGui_Decorations( custom_framebuffer );
				}
			};

			DrawFramebufferImGui( framebuffer_shadow_map_light_directional );
			DrawFramebufferImGui( framebuffer_main );
			DrawFramebufferImGui( framebuffer_postprocessing );
			DrawFramebufferImGui( framebuffer_final );

			for( auto& custom_framebuffer : framebuffer_custom_array )
				DrawCustomFramebufferImGui( custom_framebuffer );
		}

		ImGui::End();

		/* Shaders: */
		for( auto& shader : shaders_registered )
			ImGuiDrawer::Draw( *shader );

		/* Materials: */
		ImGuiDrawer::Draw( msaa_resolve_material, *this );
		ImGuiDrawer::Draw( tone_mapping_material, *this );

		/* Uniforms (Renderer-scope): */
		ImGuiDrawer::Draw( uniform_buffer_management_intrinsic, "Shader Intrinsics" );
		ImGuiDrawer::Draw( uniform_buffer_management_global,	"Shader Globals" );
	}

	void Renderer::OnFramebufferResize( const int new_width_in_pixels, const int new_height_in_pixels )
	{
		glViewport( 0, 0, new_width_in_pixels, new_height_in_pixels );

		if( shaders_need_uniform_buffer_other )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_VIEWPORT_SIZE", Vector2( ( float )new_width_in_pixels, ( float )new_height_in_pixels ) );
		}

		/* Shadow maps: */
		if( framebuffer_shadow_map_light_directional.IsValid() )
			framebuffer_shadow_map_light_directional.Resize( new_width_in_pixels, new_height_in_pixels );
		else // i.e, the first time the framebuffer is created:
		{
			framebuffer_shadow_map_light_directional = Framebuffer( Framebuffer::Description
																	{
																		.name = "Shadow Map FB - Dir. Light",

																		.width_in_pixels  = new_width_in_pixels,
																		.height_in_pixels = new_height_in_pixels,

																		.minification_filter  = Texture::Filtering::Nearest,
																		.magnification_filter = Texture::Filtering::Nearest,

																		/* Default wrapping = clamp to border, with border = Color4{0,0,0,0}. */

																		/* Default color format = RGBA, adequate. */

																		.attachment_bits = Framebuffer::AttachmentType::Depth
																	} );

		}


		/* Main: */
		if( framebuffer_main.IsValid() )
			framebuffer_main.Resize( new_width_in_pixels, new_height_in_pixels );
		else // i.e, the first time the framebuffer is created:
		{
			framebuffer_main = Framebuffer( Framebuffer::Description
											{
												.name = "Main FB",

												.width_in_pixels  = new_width_in_pixels,
												.height_in_pixels = new_height_in_pixels,

												.color_format    = framebuffer_main_color_format,
												.attachment_bits = Framebuffer::AttachmentType::Color_DepthStencilCombined,
												.msaa            = MSAA( framebuffer_main_msaa_sample_count )
											} );
		}

		/* Post-processing: */
		if( framebuffer_postprocessing.IsValid() )
			framebuffer_postprocessing.Resize( new_width_in_pixels, new_height_in_pixels );
		else // i.e, the first time the framebuffer is created:
		{
			/* Same parameters as the main FBO. */
			framebuffer_postprocessing = Framebuffer( Framebuffer::Description
													  {
														  .name = "Post-processing FB",

														  .width_in_pixels  = new_width_in_pixels,
														  .height_in_pixels = new_height_in_pixels,

														  .color_format    = framebuffer_main_color_format,
														  .attachment_bits = Framebuffer::AttachmentType::Color_DepthStencilCombined
													  } );
		}

		/* Editor: */
		if( framebuffer_final.IsValid() )
			framebuffer_final.Resize( new_width_in_pixels, new_height_in_pixels );
		else // i.e, the first time the framebuffer is created:
		{
			framebuffer_final = Framebuffer( Framebuffer::Description
											 {
												 .name = "Editor FB",

												 .width_in_pixels  = new_width_in_pixels,
												 .height_in_pixels = new_height_in_pixels,

												 .color_format    = Texture::Format::SRGBA, /* This is the final step, so sRGB encoding should be on. */
												 .attachment_bits = Framebuffer::AttachmentType::Color_DepthStencilCombined // Some post-processing fx may need depth.
											 } );
		}

		// TODO: Create a SizeInfo struct to store absolute size vs relative sizes (multipliers). For example, a rear-view mirror framebuffer may have half-resolution.
		// So resizing the main framebuffer should resize this rear-view framebuffer to half the new resolution (size info of this framebuffer will have a relative 0.5 multiplier).

		// TODO: Create APIs for the client code to modify custom framebuffer descriptions.

		/* Custom Framebuffers: */
		for( auto index = 0; index < framebuffer_custom_array.size(); index++ )
		{
			if( const auto description = framebuffer_custom_description_array[ index ]; description )
			{
				if( auto& custom_framebuffer = framebuffer_custom_array[ index ];
					custom_framebuffer.IsValid() )
					custom_framebuffer.Resize( new_width_in_pixels, new_height_in_pixels );
				else // i.e, the first time the framebuffer is created:
				{
					if( description->width_in_pixels == 0 || description->height_in_pixels == 0 )
					{
						auto copy             = *description;
						copy.width_in_pixels  = new_width_in_pixels;
						copy.height_in_pixels = new_height_in_pixels;

						custom_framebuffer = Engine::Framebuffer( copy );
					}
					else
						custom_framebuffer = Engine::Framebuffer( *description );
				}
			}
		}

		msaa_resolve_material.SetTexture( "uniform_texture_slot", &framebuffer_main.ColorAttachment() );
		tone_mapping_material.SetTexture( "uniform_texture_slot", &framebuffer_postprocessing.ColorAttachment() );
	}

	void Renderer::AddRenderable( Renderable* renderable_to_add, const RenderQueue::ID queue_id )
	{
		auto& queue = render_queue_map[ queue_id ];

		queue.renderable_list.push_back( renderable_to_add );

		const auto& shader = renderable_to_add->material->shader;

		queue.shaders_in_flight[ shader->name ] = shader;

		if( ++queue.shader_reference_counts[ shader ] == 1 )
		{
			if( not shaders_registered.contains( shader ) )
				RegisterShader( *shader );
		}

		queue.materials_in_flight.try_emplace( renderable_to_add->material->Name(), renderable_to_add->material );
	}

	void Renderer::RemoveRenderable( Renderable* renderable_to_remove )
	{
		auto& queues_containing_renderable = RenderQueuesContaining( renderable_to_remove );
		for( auto& queue : queues_containing_renderable )
		{
			// For now, stick to removing elements from a vector, which is sub-par performance but should be OK for the time being.
			std::erase( queue->renderable_list, renderable_to_remove );

			const auto& shader = renderable_to_remove->material->shader;

			if( --queue->shader_reference_counts[ shader ] == 0 )
			{
				queue->shaders_in_flight.erase( shader->name );
				queue->shader_reference_counts.erase( shader );
				if( --shaders_registered_reference_count_map[ shader ] == 0 )
					UnregisterShader( *shader );
			}

			queue->materials_in_flight.erase( renderable_to_remove->material->Name() );
		}
	}

	void Renderer::OnShaderReassign( Shader* previous_shader, const std::string& name_of_material_whose_shader_changed )
	{
		for( auto& [ queue_id, queue ] : render_queue_map )
		{
			if( auto iterator = queue.materials_in_flight.find( name_of_material_whose_shader_changed );
				iterator != queue.materials_in_flight.cend() )
			{
				Material* material = iterator->second;

				if( const auto ref_count = queue.shader_reference_counts[ previous_shader ];
					ref_count == 1 )
				{
					queue.shader_reference_counts.erase( previous_shader );
					queue.shaders_in_flight.erase( previous_shader->name );
				}

				queue.shader_reference_counts[ material->shader ]++;
				queue.shaders_in_flight[ material->shader->name ] = material->shader;

				/* Log warning if the new shader is incompatible with mesh(es). */
				for( auto& renderable : queue.renderable_list )
					if( renderable->material == material &&
						not renderable->mesh->IsCompatibleWith( renderable->material->shader->GetSourceVertexLayout() ) )
						logger.Warning( "Mesh \"" + renderable->mesh->Name() + "\" is not compatible with its current shader \"" + material->shader->Name() + "\"." );
			}
		}
	}

	RenderState& Renderer::GetRenderState( const RenderPass::ID pass_id_to_fetch )
	{
		return render_pass_map[ pass_id_to_fetch ].render_state;
	}

	void Renderer::AddPass( const RenderPass::ID new_pass_id, RenderPass&& new_pass )
	{
		if( not render_pass_map.try_emplace( new_pass_id, std::move( new_pass ) ).second )
			CONSOLE_ERROR( "Attempting to add a new pass with the ID of an existing queue!" );
	}

	void Renderer::RemovePass( const RenderPass::ID pass_id_to_remove )
	{
		if( std::find( BUILTIN_PASS_ID_LIST.cbegin(), BUILTIN_PASS_ID_LIST.cend(), pass_id_to_remove ) != BUILTIN_PASS_ID_LIST.cend() )
		{
			CONSOLE_ERROR( "Removing a built-in pass is not allowed! Please use TogglePass( id, false ) to disable unwanted passes instead." );
			return;
		}

#ifdef _DEBUG
		if( const auto iterator = render_pass_map.find( pass_id_to_remove );
			iterator != render_pass_map.cend() )
			render_pass_map.erase( pass_id_to_remove );
		else
			throw std::runtime_error( "Attempting to remove a non-existing pass!" );
#else
		render_pass_map.erase( pass_id_to_remove );
#endif // _DEBUG
	}

	void Renderer::TogglePass( const RenderPass::ID pass_id_to_toggle, const bool enable )
	{
#ifdef _EDITOR
		if( auto iterator = render_pass_map.find( pass_id_to_toggle );
			iterator != render_pass_map.cend() )
		{
			iterator->second.is_enabled = enable;
		}
		else
			CONSOLE_ERROR( "Attempting to toggle a non-existing pass!" );
#else
		render_pass_map[ pass_id_to_toggle ].is_enabled = enable;
#endif // EDITOR
	}

	bool Renderer::PassHasContentToRender( const RenderPass& pass_to_query ) const
	{
		if( not pass_to_query.is_enabled )
			return false;

		for( auto& queue_id : pass_to_query.queue_id_set )
			if( const auto& queue = render_queue_map.at( queue_id );
				queue.is_enabled && QueueHasContentToRender( queue ) )
				return true;

		return false;
	}

	void Renderer::AddQueue( const RenderQueue::ID new_queue_id, RenderQueue&& new_queue )
	{
		if( not render_queue_map.try_emplace( new_queue_id, std::move( new_queue ) ).second )
			CONSOLE_ERROR( "Attempting to add a new queue with the ID of an existing queue!" );
	}

	void Renderer::RemoveQueue( const RenderQueue::ID queue_id_to_remove )
	{
		if( std::find( BUILTIN_QUEUE_ID_LIST.cbegin(), BUILTIN_QUEUE_ID_LIST.cend(), queue_id_to_remove ) != BUILTIN_QUEUE_ID_LIST.cend() )
		{
			CONSOLE_ERROR( "Removing a built-in queue is not allowed! Please use ToggleQueue( id, false ) to disable unwanted passes instead." );
			return;
		}

#ifdef _DEBUG
		if( const auto iterator = render_queue_map.find( queue_id_to_remove );
			iterator != render_queue_map.cend() )
			render_queue_map.erase( queue_id_to_remove );
		else
			throw std::runtime_error( "Attempting to remove a non-existing queue!" );
	#else
		render_queue_map.erase( queue_id_to_remove );
#endif // _DEBUG
	}

	void Renderer::ToggleQueue( const RenderQueue::ID queue_id_to_toggle, const bool enable )
	{
	#ifdef _EDITOR
		if( auto iterator = render_queue_map.find( queue_id_to_toggle );
			iterator != render_queue_map.cend() )
		{
			iterator->second.is_enabled = enable;
		}
		else
			CONSOLE_ERROR( "Attempting to toggle a non-existing queue!" );
	#else
		render_queue_map[ queue_id_to_toggle ].is_enabled = enable;
	#endif // _EDITOR
	}

	bool Renderer::QueueHasContentToRender( const RenderQueue& queue_to_query ) const
	{
		if( queue_to_query.is_enabled && not queue_to_query.renderable_list.empty() )
			for( auto& renderable : queue_to_query.renderable_list )
				if( renderable && renderable->is_enabled )
					return true;

		return false;
	}

	void Renderer::AddQueueToPass( const RenderQueue::ID queue_id_to_add, const RenderPass::ID pass_to_add_to )
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "AddQueueToPass", pass_to_add_to );
		CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( "AddQueueToPass", queue_id_to_add );

		if( not render_pass_map[ pass_to_add_to ].queue_id_set.emplace( queue_id_to_add ).second )
			CONSOLE_ERROR( "Attempting to add an already existing queue to a pass!" );

	}
	void Renderer::RemoveQueueFromPass( const RenderQueue::ID queue_id_to_remove, const RenderPass::ID pass_to_remove_from )
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "RemoveQueueFromPass", pass_to_remove_from );
		CONSOLE_ERROR_AND_RETURN_IF_QUEUE_DOES_NOT_EXIST( "RemoveQueueFromPass", queue_id_to_remove );

		if( not render_pass_map[ pass_to_remove_from ].queue_id_set.erase( queue_id_to_remove ) )
			CONSOLE_ERROR( "Attempting to add a non-existing queue from a pass!" );
	}

	void Renderer::SetFinalPassToUseFinalFramebuffer()
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "SetFinalPassToUseEditorFramebuffer", PASS_ID_FINAL );

		auto& pass = render_pass_map[ PASS_ID_FINAL ];
		pass.target_framebuffer = &framebuffer_final;
	}

	void Renderer::SetFinalPassToUseDefaultFramebuffer()
	{
		CONSOLE_ERROR_AND_RETURN_IF_PASS_DOES_NOT_EXIST( "SetFinalPassToUseDefaultFramebuffer", PASS_ID_FINAL );

		auto& pass = render_pass_map[ PASS_ID_FINAL ];
		pass.target_framebuffer = nullptr;
	}

	void Renderer::AddDirectionalLight( DirectionalLight* light_to_add )
	{
		if( light_directional )
			throw std::runtime_error( "Only 1 Directional Light can be active at a time!" );

		light_directional = light_to_add;
	}

	void Renderer::RemoveDirectionalLight()
	{
		if( !light_directional )
			throw std::runtime_error( "No Directional Light set yet!" );

		light_directional = nullptr;
	}

	void Renderer::AddPointLight( PointLight* light_to_add )
	{
		lights_point.push_back( light_to_add );
	}

	void Renderer::RemovePointLight( PointLight* light_to_remove )
	{
		std::erase( lights_point, light_to_remove );
	}

	void Renderer::RemoveAllPointLights()
	{
		lights_point.clear();
	}

	void Renderer::AddSpotLight( SpotLight* light_to_add )
	{
		lights_spot.push_back( light_to_add );
	}

	void Renderer::RemoveSpotLight( SpotLight* light_to_remove )
	{
		std::erase( lights_spot, light_to_remove );
	}

	void Renderer::RemoveAllSpotLights()
	{
		lights_spot.clear();
	}

	const void* Renderer::GetShaderGlobal( const std::string& buffer_name ) const
	{
		return uniform_buffer_management_global.Get( buffer_name );
	}

	void* Renderer::GetShaderGlobal( const std::string& buffer_name )
	{
		return uniform_buffer_management_global.Get( buffer_name );
	}

	void Renderer::RegisterShader( Shader& shader )
	{
		if( shader.HasUniformBlocks() )
		{
			/* Regular Uniform Buffers are handled by the Material class.
			 * Globals & Intrinsics are registered here. */

			if( shader.HasGlobalUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Global();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_global.RegisterBuffer_ForceUpdateBufferInfoIfBufferExists( uniform_buffer_name, uniform_buffer_info );
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
					uniform_buffer_management_intrinsic.RegisterBuffer_ForceUpdateBufferInfoIfBufferExists( uniform_buffer_name, uniform_buffer_info );
			}
		}

		if( shader.GetUniformBufferInfoMap_Intrinsic().contains( "_Intrinsic_Lighting" ) )
		{
			shaders_using_intrinsics_lighting.insert( &shader );
			shaders_need_uniform_buffer_lighting = true;
		}

		if( shader.GetUniformBufferInfoMap_Intrinsic().contains( "_Intrinsic_Other" ) )
		{
			shaders_using_intrinsics_other.insert( &shader );
			shaders_need_uniform_buffer_other = true;
		}

		if( ++shaders_registered_reference_count_map[ &shader ] == 1 )
			shaders_registered.insert( &shader );
	}

	void Renderer::UnregisterShader( Shader& shader )
	{
		if( shader.HasUniformBlocks() )
		{
			/* Regular Uniform Buffers are handled by the Material class.
			 * Globals & Intrinsics are registered here. */

			if( shader.HasGlobalUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Global();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				{
					// Check if this buffer is still used by other registered Shaders:
					if( std::find_if( shaders_registered.cbegin(), shaders_registered.cend(),
										[ &uniform_buffer_name ]( const Shader* shader ) { return shader->HasGlobalUniformBlock( uniform_buffer_name ); } ) == shaders_registered.cend() )
						uniform_buffer_management_global.UnregisterBuffer( uniform_buffer_name );
				}
			}

			if( shader.HasIntrinsicUniformBlocks() )
			{
				const auto& uniform_buffer_info_map = shader.GetUniformBufferInfoMap_Intrinsic();

				for( auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				{
					// Check if this buffer is still used by other registered Shaders:
					if( std::find_if( shaders_registered.cbegin(), shaders_registered.cend(),
										[ &uniform_buffer_name ]( const Shader* shader ) { return shader->HasIntrinsicUniformBlock( uniform_buffer_name ); } ) == shaders_registered.cend() )
						uniform_buffer_management_intrinsic.UnregisterBuffer( uniform_buffer_name );
				}
			}
		}

		if( auto iterator = shaders_using_intrinsics_lighting.find( &shader );
			iterator != shaders_using_intrinsics_lighting.cend() )
		{
			shaders_using_intrinsics_lighting.erase( iterator );
			shaders_need_uniform_buffer_lighting = not shaders_using_intrinsics_lighting.empty();
		}

		if( auto iterator = shaders_using_intrinsics_other.find( &shader );
			iterator != shaders_using_intrinsics_other.cend() )
		{
			shaders_using_intrinsics_other.erase( iterator );
			shaders_need_uniform_buffer_other = not shaders_using_intrinsics_other.empty();
		}

		shaders_registered.erase( &shader );
		shaders_registered_reference_count_map.erase( &shader );
	}

/*
 * 
 *	PRIVATE API:
 * 
 */

	void Renderer::EnableStencilTest()
	{
		glEnable( GL_STENCIL_TEST );
	}

	void Renderer::DisableStencilTest()
	{
		glDisable( GL_STENCIL_TEST );
	}

	void Renderer::SetStencilWriteMask( const unsigned int mask )
	{
		glStencilMask( mask );
	}

	void Renderer::SetStencilTestResponses( const StencilTestResponse stencil_fail, const StencilTestResponse stencil_pass_depth_fail, const StencilTestResponse both_pass )
	{
		glStencilOp( ( GLenum )stencil_fail, ( GLenum )stencil_pass_depth_fail, ( GLenum )both_pass );
	}

	void Renderer::SetStencilComparisonFunction( const ComparisonFunction comparison_function, const int reference_value, const unsigned int mask )
	{
		glStencilFunc( ( GLenum )comparison_function, reference_value, mask );
	}

	void Renderer::EnableDepthTest()
	{
		glEnable( GL_DEPTH_TEST );
	}

	void Renderer::DisableDepthTest()
	{
		glDisable( GL_DEPTH_TEST );
	}

	void Renderer::ToggleDepthWrite( const bool enable )
	{
		glDepthMask( ( GLint )enable );
	}

	void Renderer::SetDepthComparisonFunction( const ComparisonFunction comparison_function )
	{
		glDepthFunc( ( GLenum )comparison_function );
	}

	void Renderer::EnableBlending()
	{
		glEnable( GL_BLEND );
	}

	void Renderer::DisableBlending()
	{
		glDisable( GL_BLEND );
	}

	void Renderer::SetBlendingFactors( const BlendingFactor source_color_factor, const BlendingFactor destination_color_factor,
									   const BlendingFactor source_alpha_factor, const BlendingFactor destination_alpha_factor )
	{
		glBlendFuncSeparate( ( GLenum )source_color_factor, ( GLenum )destination_color_factor, ( GLenum )source_alpha_factor, ( GLenum )destination_alpha_factor );
	}

	void Renderer::SetBlendingFunction( const BlendingFunction function )
	{
		glBlendEquation( ( GLenum )function );
	}

	void Renderer::SetCurrentFramebuffer( Framebuffer* framebuffer )
	{
		ASSERT_DEBUG_ONLY( framebuffer && ICON_FA_DRAW_POLYGON " " "SetCurrentFramebuffer() called with nullptr. ResetToDefaultFramebuffer() can be used to bind the default framebuffer." );

		const Vector2I old_framebuffer_size = framebuffer_current ? framebuffer_current->Size() : Vector2I{};
		
		framebuffer_current = framebuffer;
		framebuffer_current->Bind();

		if( framebuffer->Size() != old_framebuffer_size )
			glViewport( 0, 0, framebuffer->Width(), framebuffer->Height() );
	}

	void Renderer::ResetToDefaultFramebuffer( const Framebuffer::BindPoint bind_point )
	{
		framebuffer_current = nullptr;
		DefaultFramebuffer::Bind( bind_point );
	}

	bool Renderer::DefaultFramebufferIsBound() const
	{
		return framebuffer_current == nullptr;
	}

	Framebuffer* Renderer::CurrentFramebuffer()
	{
		return framebuffer_current;
	}

	Framebuffer& Renderer::MainFramebuffer()
	{
		return framebuffer_main;
	}

	Framebuffer& Renderer::PostProcessingFramebuffer()
	{
		return framebuffer_postprocessing;
	}

	Framebuffer& Renderer::FinalFramebuffer()
	{
		return framebuffer_final;
	}

	Framebuffer& Renderer::CustomFramebuffer( const unsigned int framebuffer_index )
	{
		return framebuffer_custom_array[ framebuffer_index ];
	}

	void Renderer::EnableFramebuffer_sRGBEncoding()
	{
		glEnable( GL_FRAMEBUFFER_SRGB );
		framebuffer_sRGB_encoding_is_enabled = true;
	}

	void Renderer::DisableFramebuffer_sRGBEncoding()
	{
		glDisable( GL_FRAMEBUFFER_SRGB );
		framebuffer_sRGB_encoding_is_enabled = false;
	}

	bool Renderer::CheckMSAASupport( const Texture::Format format, const std::uint8_t sample_count_to_query )
	{
		static std::unordered_map< Texture::Format, std::unordered_set< std::uint8_t > > SAMPLE_COUNTS_BY_FORMAT_MAP;

		if( const auto iterator = SAMPLE_COUNTS_BY_FORMAT_MAP.find( format ); iterator != SAMPLE_COUNTS_BY_FORMAT_MAP.cend() )
			return iterator->second.contains( sample_count_to_query );

		int number_of_sample_counts = 0;
		glGetInternalformativ( GL_RENDERBUFFER, Texture::InternalFormat( format ), GL_NUM_SAMPLE_COUNTS, 1, &number_of_sample_counts );

		std::vector< int > sample_counts( number_of_sample_counts );
		glGetInternalformativ( GL_RENDERBUFFER, Texture::InternalFormat( format ), GL_SAMPLES, number_of_sample_counts, sample_counts.data() );
		auto& set_of_sample_counts_queried = SAMPLE_COUNTS_BY_FORMAT_MAP[ format ];
		std::transform( sample_counts.begin(), sample_counts.end(), std::inserter( set_of_sample_counts_queried, set_of_sample_counts_queried.begin() ),
						[]( int sample_count ) { return sample_count; } );

		return set_of_sample_counts_queried.contains( sample_count_to_query );
	}

	void Renderer::SetPolygonMode( const PolygonMode mode )
	{
		glPolygonMode( GL_FRONT_AND_BACK, ( GLenum )mode );
	}

	void Renderer::InitializeBuiltinQueues()
	{
		AddQueue( QUEUE_ID_GEOMETRY,
				  RenderQueue
				  {
					  .name                  = "Geometry",
					  .render_state_override = RenderState
					  {
						  .sorting_mode = SortingMode::FrontToBack
					  }
				  } );

		AddQueue( QUEUE_ID_TRANSPARENT,
				  RenderQueue
				  {
					  .name                  = "Transparent",
					  .render_state_override = RenderState
					  {
						  .blending_enable = true,

						  .sorting_mode = Engine::SortingMode::BackToFront,

						  .blending_source_color_factor      = BlendingFactor::SourceAlpha,
						  .blending_destination_color_factor = BlendingFactor::OneMinusSourceAlpha,
						  .blending_source_alpha_factor      = BlendingFactor::SourceAlpha,
						  .blending_destination_alpha_factor = BlendingFactor::OneMinusSourceAlpha
					  }
				  } );

		AddQueue( QUEUE_ID_SKYBOX,
				  RenderQueue
				  {
					  .name                  = "Skybox",
					  .render_state_override = RenderState
					  {
						 .face_culling_enable = false,

						 .depth_comparison_function = ComparisonFunction::LessOrEqual
					  }
				  } );

		AddQueue( QUEUE_ID_MSAA_RESOLVE,
				  RenderQueue
				  {
					  .name = "MSAA Resolve",
					  .render_state_override = RenderState
					  {
						  .face_culling_enable = false,

						  .depth_test_enable  = false,
						  .depth_write_enable = false,

						  .sorting_mode = SortingMode::None // Preserve insertion order.
					  }
				  } );

		AddQueue( QUEUE_ID_POSTPROCESSING,
				  RenderQueue
				  {
					  .name = "Post-processing",
					  .render_state_override = RenderState
					  {
						  .face_culling_enable = false,

				          .depth_test_enable  = false,
						  .depth_write_enable = false,

						  .sorting_mode = SortingMode::None // Preserve effect insertion order.
                      }
				  } );

		AddQueue( QUEUE_ID_FINAL,
				  RenderQueue
				  {
					  .name = "Final",
					  .render_state_override = RenderState
					  {
						  .face_culling_enable = false,

						  .depth_test_enable  = false,
						  .depth_write_enable = false,

						  .sorting_mode = SortingMode::None // Preserve insertion order.
					  }
				  } );
	}

	void Renderer::InitializeBuiltinPasses()
	{
		AddPass( PASS_ID_SHADOW_MAPPING,
				 RenderPass
				 {
					 .name               = "Shadow Mapping",
					 .target_framebuffer = &framebuffer_shadow_map_light_directional,
					 .queue_id_set       = { QUEUE_ID_GEOMETRY },
					 .view_matrix		 = Matrix4x4{},
					 .projection_matrix  = Matrix4x4{},
					 .render_state       = RenderState
					 {
						 .sorting_mode = SortingMode::FrontToBack,
					 },
					 .render_state_override_is_allowed = false,
					 .clear_framebuffer                = true
				 } );

		AddPass( PASS_ID_LIGHTING,
				 RenderPass
				 {
					 .name               = "Lighting",
					 .target_framebuffer = &framebuffer_main,
					 .queue_id_set       = { QUEUE_ID_GEOMETRY, QUEUE_ID_TRANSPARENT, QUEUE_ID_SKYBOX },
					 .clear_framebuffer  = true,
				 } );

		AddPass( PASS_ID_MSAA_RESOLVE,
				 RenderPass
				 {
					 .name               = "MSAA Resolve",
					 .target_framebuffer = &framebuffer_postprocessing,
					 .queue_id_set       = { QUEUE_ID_MSAA_RESOLVE }
				 } );

		AddPass( PASS_ID_POSTPROCESSING,
				 RenderPass
				 {
					 .name               = "Post-processing",
					 .target_framebuffer = &framebuffer_postprocessing,
					 .queue_id_set       = { QUEUE_ID_POSTPROCESSING }
				 } );

		AddPass( PASS_ID_FINAL,
				 RenderPass
				 {
					 .name               = "Final",
					 .target_framebuffer = &framebuffer_final,
					 .queue_id_set       = { QUEUE_ID_FINAL }
				 } );
	}

	void Renderer::Render( const Mesh& mesh )
	{
		mesh.HasInstancing()
			? mesh.HasIndices()
				? RenderInstanced_Indexed( mesh )
				: RenderInstanced_NonIndexed( mesh )
			: mesh.HasIndices()
				? Render_Indexed( mesh )
				: Render_NonIndexed( mesh );
	}

	void Renderer::Render_Indexed( const Mesh& mesh )
	{
		glDrawElements( ( GLint )mesh.Primitive(), mesh.IndexCount(), mesh.IndexType(), 0 );
	}

	void Renderer::Render_NonIndexed( const Mesh& mesh )
	{
		glDrawArrays( ( GLint )mesh.Primitive(), 0, mesh.VertexCount() );
	}

	void Renderer::RenderInstanced( const Mesh& mesh )
	{
		mesh.HasIndices()
			? RenderInstanced_Indexed( mesh )
			: RenderInstanced_NonIndexed( mesh );
	}

	void Renderer::RenderInstanced_Indexed( const Mesh& mesh )
	{
		glDrawElementsInstanced( ( GLint )mesh.Primitive(), mesh.IndexCount(), mesh.IndexType(), 0, mesh.InstanceCount() );
	}

	void Renderer::RenderInstanced_NonIndexed( const Mesh& mesh )
	{
		glDrawArraysInstanced( ( GLint )mesh.Primitive(), 0, mesh.VertexCount(), mesh.InstanceCount() );
	}

	void Renderer::SetIntrinsicsPerPass( const RenderPass& pass )
	{
		/* Update view/projection matrix, only if dirty:
		 * Lighting is always updated, regardless of dirty state. That's because I don't want to deal with it right now. */
		BitFlags< IntrinsicModifyTarget > intrinsic_modification_targets( IntrinsicModifyTarget::UniformBuffer_Lighting );

		if( pass.view_matrix && pass.view_matrix != current_camera_info.view_matrix )
		{
			intrinsic_modification_targets.Set( IntrinsicModifyTarget::UniformBuffer_View );
			current_camera_info.view_matrix = *pass.view_matrix;
		}

		if( pass.projection_matrix && pass.projection_matrix != current_camera_info.projection_matrix )
		{
			intrinsic_modification_targets.Set( IntrinsicModifyTarget::UniformBuffer_Projection );
			current_camera_info.projection_matrix      = *pass.projection_matrix;
			current_camera_info.plane_near             = pass.plane_near;
			current_camera_info.plane_far              = pass.plane_far;
			current_camera_info.aspect_ratio           = pass.aspect_ratio;
			current_camera_info.vertical_field_of_view = pass.vertical_field_of_view;
		}

		if( intrinsic_modification_targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) ||
			intrinsic_modification_targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Projection ) )
		{
			current_camera_info.view_projection_matrix = current_camera_info.view_matrix * current_camera_info.projection_matrix;
		}

		SetIntrinsics( intrinsic_modification_targets );
	}

	void Renderer::SetIntrinsics( const BitFlags< IntrinsicModifyTarget > targets )
	{
		if( targets == IntrinsicModifyTarget::None )
			return;

		if( shaders_need_uniform_buffer_other && targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Projection ) )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_PROJECTION", current_camera_info.projection_matrix );
			if( not targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) ) // No need to upload twice.
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_PROJECTION", current_camera_info.view_projection_matrix );

			if( Matrix::IsPerspectiveProjection( current_camera_info.projection_matrix ) ) // No need to upload these if they will not mean anything anyway.
			{
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_NEAR",					current_camera_info.plane_near );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_FAR",					current_camera_info.plane_far );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_ASPECT_RATIO",			current_camera_info.aspect_ratio );
				uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_PROJECTION_VERTICAL_FIELD_OF_VIEW", current_camera_info.vertical_field_of_view );
			}
		}

		const auto& view_matrix               = current_camera_info.view_matrix;
		const auto& view_matrix_3x3           = view_matrix.SubMatrix< 3 >();
		const auto& view_matrix_rotation_only = Matrix4x4( view_matrix_3x3 );

		if( shaders_need_uniform_buffer_other && targets.IsSet( IntrinsicModifyTarget::UniformBuffer_View ) )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW",				view_matrix );
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_ROTATION_ONLY",	view_matrix_rotation_only );
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Other", "_INTRINSIC_TRANSFORM_VIEW_PROJECTION",		current_camera_info.view_projection_matrix );
		}

		if( shaders_need_uniform_buffer_lighting && targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Lighting ) )
		{
			/* Lighting is always updated, regardless of dirty state. That's because I don't want to deal with it right now. */

			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_DIRECTIONAL_LIGHT_IS_ACTIVE", 
															light_directional && light_directional->is_enabled ? 1u : 0u );
			if( light_directional && light_directional->is_enabled )
			{
				light_directional->data.direction_view_space = light_directional->transform->Forward() * view_matrix_3x3;
				uniform_buffer_management_intrinsic.SetPartial_Struct( "_Intrinsic_Lighting", "_INTRINSIC_DIRECTIONAL_LIGHT", light_directional->data );
			}

			lights_point_active_count = 0;
			for( auto index = 0; index < lights_point.size(); index++ )
			{
				auto& point_light = lights_point[ index ];

				if( point_light->is_enabled )
				{
					/* Shaders expect the lights' position & direction in view space. */
					point_light->data.position_view_space = Vector4( point_light->transform->GetTranslation() ).SetW( 1.0f ) * view_matrix;
					uniform_buffer_management_intrinsic.SetPartial_Array( "_Intrinsic_Lighting", "_INTRINSIC_POINT_LIGHTS", lights_point_active_count++, point_light->data );
				}
			}
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_POINT_LIGHT_ACTIVE_COUNT", lights_point_active_count );

			lights_spot_active_count = 0;
			for( auto index = 0; index < lights_spot.size(); index++ )
			{
				auto& spot_light = lights_spot[ index ];

				if( spot_light->is_enabled )
				{
					/* Shaders expect the lights' position & direction in view space. */

					spot_light->data.position_view_space_and_cos_cutoff_angle_inner.vector = ( Vector4( spot_light->transform->GetTranslation() ).SetW( 1.0f ) * view_matrix ).XYZ();
					spot_light->data.position_view_space_and_cos_cutoff_angle_inner.scalar = Math::Cos( Radians( spot_light->data.cutoff_angle_inner ) );

					spot_light->data.direction_view_space_and_cos_cutoff_angle_outer.vector = spot_light->transform->Forward() * view_matrix_3x3;
					spot_light->data.direction_view_space_and_cos_cutoff_angle_outer.scalar = Math::Cos( Radians( spot_light->data.cutoff_angle_outer ) );

					uniform_buffer_management_intrinsic.SetPartial_Array( "_Intrinsic_Lighting", "_INTRINSIC_SPOT_LIGHTS", lights_spot_active_count++, spot_light->data );
				}
			}
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_SPOT_LIGHT_ACTIVE_COUNT", lights_spot_active_count );
		}

		if( shaders_need_uniform_buffer_lighting && targets.IsSet( IntrinsicModifyTarget::UniformBuffer_Lighting_ShadowMapping ) )
		{
			uniform_buffer_management_intrinsic.SetPartial( "_Intrinsic_Lighting", "_INTRINSIC_DIRECTIONAL_LIGHT_VIEW_PROJECTION_TRANSFORM", 
															light_directional_view_projection_transform_matrix );
		}
	}

	void Renderer::UploadIntrinsics()
	{
		uniform_buffer_management_intrinsic.UploadAll();
	}

	void Renderer::UploadGlobals()
	{
		uniform_buffer_management_global.UploadAll();
	}

	void Renderer::CalculateShadowMappingInformation()
	{
		if( light_directional == nullptr )
			return;

		Transform light_transform_copy( *light_directional->transform );

		const auto view_matrix = light_transform_copy.GetInverseOfFinalMatrix_NoScale();

		// TODO: AABBs.
		// TODO: Render the projection volume as a wireframe cube. More generally, create a debug-render pass, with wire-frame rendering.
		// Use ortho. projection for directional light. For now, use big values for the projection volume:
		const auto projection_matrix = Matrix::OrthographicProjection( -50.0f, +50.0f, -50.0f, +50.0f, 1.0f, 101.0f );

		auto& shadow_mapping_pass = render_pass_map[ PASS_ID_SHADOW_MAPPING ];
		
		bool recalculate_view_projection_matrix = false;

		if( shadow_mapping_pass.view_matrix != view_matrix )
		{
			shadow_mapping_pass.view_matrix = view_matrix;
			recalculate_view_projection_matrix = true;
		}

		if( shadow_mapping_pass.projection_matrix != projection_matrix )
		{
			shadow_mapping_pass.projection_matrix = projection_matrix;
			recalculate_view_projection_matrix = true;
		}

		if( recalculate_view_projection_matrix )
		{
			light_directional_view_projection_transform_matrix = view_matrix * projection_matrix;
			SetIntrinsics( IntrinsicModifyTarget::UniformBuffer_Lighting_ShadowMapping );
		}
	}

	void Renderer::RecompileModifiedShaders()
	{
		/* Shader Recompilation: */
		static std::vector< Shader* > shaders_to_recompile;

		/* Have to do two passes as shaders to be recompiled need to be removed from shaders_registered, which we can not do while traversing the container. */

		shaders_to_recompile.clear();

		for( const auto& shader : shaders_registered )
			if( shader->SourceFilesAreModified() )
				shaders_to_recompile.push_back( shader );

		for( auto& shader : shaders_to_recompile )
		{
			Shader new_shader( shader->name.c_str() );
			if( shader->RecompileFromThis( new_shader ) )
			{
				UnregisterShader( *shader );

				*shader = std::move( new_shader );

				RegisterShader( *shader );

				logger.Info( "\"" + shader->name + "\" shader's source files are modified. It is recompiled." );
			}
			else
				logger.Error( "\"" + shader->name + "\" shader's source files are modified but it could not be recompiled successfully." );
		}
	}

	void Renderer::InitializeInternalMeshes()
	{
		full_screen_quad_mesh = Mesh( Primitive::NonIndexed::Quad_FullScreen::Positions,
									  "[Renderer] Quad (FullScreen)",
									  { /* No normals. */ },
									  Primitive::NonIndexed::Quad_FullScreen::UVs,
									  { /* No indices. */ } );
	}

	void Renderer::InitializeInternalShaders()
	{
		shader_msaa_resolve = BuiltinShaders::Get( "MSAA Resolve" );
		shader_tone_mapping = BuiltinShaders::Get( "Tone Mapping" );
	}

	void Renderer::InitializeInternalMaterials()
	{
		// TODO: Handle No MSAA case.

		msaa_resolve_material = Material( "[Renderer] MSAA Resolve", shader_msaa_resolve );
		msaa_resolve_material.Set( "uniform_sample_count", ( int )framebuffer_main_msaa_sample_count );

		// TODO: Handle no tone mapping case.

		tone_mapping_material = Material( "[Renderer] Tone Mapping", shader_tone_mapping );
		tone_mapping_material.Set( "uniform_exposure", 1.0f );
	}

	void Renderer::InitializeInternalRenderables()
	{
		msaa_resolve_renderable = Renderable( &full_screen_quad_mesh, &msaa_resolve_material );
		AddRenderable( &msaa_resolve_renderable, QUEUE_ID_MSAA_RESOLVE );

		tone_mapping_renderable = Renderable( &full_screen_quad_mesh, &tone_mapping_material );
		AddRenderable( &tone_mapping_renderable, QUEUE_ID_FINAL );
	}

	std::vector< RenderQueue* >& Renderer::RenderQueuesContaining( const Renderable* renderable_of_interest )
	{
		static std::vector< RenderQueue* > queues;
		queues.clear();

		for( auto& [ queue_id, queue ] : render_queue_map )
			if( std::find( queue.renderable_list.cbegin(), queue.renderable_list.cend(), renderable_of_interest ) != queue.renderable_list.cend() )
				queues.push_back( &queue );

		return queues;
	}

	void Renderer::SetRenderState( const RenderState& render_state_to_set, Framebuffer* target_framebuffer, const bool clear_framebuffer )
	{
		if( framebuffer_current != target_framebuffer )
		{
			if( target_framebuffer )
				SetCurrentFramebuffer( target_framebuffer );
			else
				ResetToDefaultFramebuffer();
		}

		ToggleDepthWrite( render_state_to_set.depth_write_enable );
		SetStencilWriteMask( render_state_to_set.stencil_write_mask );

		if( clear_framebuffer )
			framebuffer_current ? framebuffer_current->Clear() : DefaultFramebuffer::Clear();

		if( const auto framebuffer_uses_srgb_encoding = framebuffer_current ? framebuffer_current->Is_sRGB() : true /* Default framebuffer always uses sRGB Encoding. */;
			framebuffer_uses_srgb_encoding != framebuffer_sRGB_encoding_is_enabled )
		{
			if( framebuffer_uses_srgb_encoding )
				EnableFramebuffer_sRGBEncoding();
			else
				DisableFramebuffer_sRGBEncoding();
		}

		if( render_state_to_set.face_culling_enable )
			EnableFaceCulling();
		else
			DisableFaceCulling();

		SetCullFace( render_state_to_set.face_culling_face_to_cull );
		SetFrontFaceConvention( render_state_to_set.face_culling_winding_order );

		if( render_state_to_set.depth_test_enable )
			EnableDepthTest();
		else
			DisableDepthTest();

		SetDepthComparisonFunction( render_state_to_set.depth_comparison_function );

		if( render_state_to_set.stencil_test_enable )
			EnableStencilTest();
		else
			DisableStencilTest();

		SetStencilComparisonFunction( render_state_to_set.stencil_comparison_function, render_state_to_set.stencil_ref, render_state_to_set.stencil_mask );
		SetStencilTestResponses( render_state_to_set.stencil_test_response_stencil_fail, 
								 render_state_to_set.stencil_test_response_stencil_pass_depth_fail, 
								 render_state_to_set.stencil_test_response_both_pass );

		if( render_state_to_set.blending_enable )
			EnableBlending();
		else
			DisableBlending();

		SetBlendingFactors( render_state_to_set.blending_source_color_factor, render_state_to_set.blending_destination_color_factor,
							render_state_to_set.blending_source_alpha_factor, render_state_to_set.blending_destination_alpha_factor );
		SetBlendingFunction( render_state_to_set.blending_function );
	}

	void Renderer::SortRenderablesInQueue( const Vector3& camera_position, std::vector< Renderable* >& renderable_array_to_sort, const SortingMode sorting_mode )
	{
		switch( sorting_mode )
		{
			case SortingMode::FrontToBack:
				std::sort( renderable_array_to_sort.begin(), renderable_array_to_sort.end(),
						   [ & ]( Renderable* renderable_1, Renderable* renderable_2 )
							{
								if( renderable_1->GetTransform() && renderable_2->GetTransform() )
									return Math::Distance( camera_position, renderable_1->GetTransform()->GetTranslation() ) <
										   Math::Distance( camera_position, renderable_2->GetTransform()->GetTranslation() );

								return false; // Does not matter;
							} );
				break;

			case SortingMode::BackToFront:
				std::sort( renderable_array_to_sort.begin(), renderable_array_to_sort.end(),
						   [ & ]( Renderable* renderable_1, Renderable* renderable_2 )
							{
								if( renderable_1->GetTransform() && renderable_2->GetTransform() )
									return Math::Distance( camera_position, renderable_1->GetTransform()->GetTranslation() ) >
										   Math::Distance( camera_position, renderable_2->GetTransform()->GetTranslation() );
							
								return false; // Does not matter;
							} );
				break;

			default:
			case SortingMode::None:
				break;
		}
	}

	void Renderer::EnableFaceCulling()
	{
		glEnable( GL_CULL_FACE );
	}

	void Renderer::DisableFaceCulling()
	{
		glDisable( GL_CULL_FACE );
	}

	void Renderer::SetCullFace( const Face face )
	{
		glCullFace( ( GLenum )face );
	}

	void Renderer::SetFrontFaceConvention( const WindingOrder winding_order_of_front_faces )
	{
		glFrontFace( ( GLenum )winding_order_of_front_faces );
	}
}
