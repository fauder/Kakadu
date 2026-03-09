// Editor Includes.
#include "RendererPanel.h"

// Engine Includes.
#include "Core/ImGuiDrawer.hpp"
#include "Core/ImGuiUtility.h"
#include "Core/ImGuiCustomColors.h"

// Vendor Includes.
#include <ImGui/imgui.h>
#include <IconFontCppHeaders/IconsFontAwesome6.h>

namespace Kakadu::Editor
{
	void RendererPanel::Render( Renderer& renderer, RendererIntrospectionSurface& introspection_surface )
	{
		const auto& style = ImGui::GetStyle();

		if( ImGui::Begin( ICON_FA_BOLT_LIGHTNING " Renderer", nullptr, ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			auto& main_framebuffer = renderer.MainFramebuffer();

			if( ImGui::BeginTabBar( "Renderer-Tab-Bar" ) )
			{
				if( ImGui::BeginTabItem( ICON_FA_DIAGRAM_PROJECT " Render Pipeline" ) )
				{
					ImGui::SeparatorText( ICON_FA_FLAG_CHECKERED " Passes & " ICON_FA_BARS " Queues" );

					if( ImGui::BeginTable( "Render Pipeline", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
					{
						ImGui::TableSetupColumn( "Name" );
						ImGui::TableSetupColumn( ICON_FA_OBJECT_GROUP " Target Framebuffer" );

						ImGui::TableNextRow( ImGuiTableRowFlags_Headers ); // Indicates that the header row will be modified.
						ImGuiUtility::Table_Header_ManuallySubmit_AppendHelpMarker( 0,
																					"A pass will not render if:\n"
																					"\t" ICON_FA_ARROW_RIGHT " All its queues are empty/disabled, \n"
																					"\t" ICON_FA_ARROW_RIGHT " AND/OR All renderables inside those queues are all disabled." );
						ImGuiUtility::Table_Header_ManuallySubmit( 1 );
						ImGui::TableNextRow();

						for( auto& [ pass_id, pass ] : *introspection_surface.render_pass_map )
						{
							ImGui::TableNextColumn();
							ImGui::PushID( ( int )pass_id );

							const bool pass_has_content_to_render = renderer.PassHasContentToRender( pass );

							if( not pass_has_content_to_render )
								ImGuiUtility::BeginDisabledButInteractable();

							ImGuiUtility::EyeCheckbox( "", &pass.is_enabled );

							ImGui::PopID();
							ImGui::SameLine();
							if( ImGui::TreeNodeEx( pass.name.c_str(), 0, ICON_FA_FLAG_CHECKERED " #%d %s", ( int )pass_id, pass.name.c_str() ) )
							{
								// TODO: Display RenderState info as a collapsible header.
								for( auto& queue_id : pass.queue_id_set )
								{
									auto& queue = ( *introspection_surface.render_queue_map )[ queue_id ];

									if( queue.renderable_list.empty() )
									{
										ImGui::TextDisabled( ICON_FA_BARS " Empty #%d %s", ( int )queue_id, queue.name.c_str() );
										continue;
									}

									const bool queue_has_content_to_render = renderer.QueueHasContentToRender( queue );

									if( not queue_has_content_to_render )
										ImGuiUtility::BeginDisabledButInteractable();

									ImGui::PushID( ( int )queue_id );
									ImGuiUtility::EyeCheckbox( "", &queue.is_enabled );
									ImGui::PopID();
									ImGui::SameLine();
									if( ImGui::TreeNodeEx( queue.name.c_str(), 0, ICON_FA_BARS " #%d %s", ( int )queue_id, queue.name.c_str() ) )
									{
										ImGui::BeginDisabled( not queue.is_enabled );

										for( const auto& [ shader_name, shader ] : queue.shaders_in_flight )
										{
											for( const auto& [ material_name, material ] : queue.materials_in_flight )
											{
												if( material->GetShader()->Id() == shader->Id() )
												{
													for( auto renderable : queue.renderable_list )
													{
														if( auto material = renderable->GetMaterial();
															material->Name() == material_name )
														{
															ImGui::PushID( renderable );
															ImGuiUtility::EyeCheckbox( "", &renderable->is_enabled );
															ImGui::PopID();
															ImGui::BeginDisabled( not renderable->is_enabled );
															ImGui::SameLine(); ImGui::TextUnformatted( material->Name().c_str() );
															if( auto mesh = renderable->GetMesh();
																mesh->HasInstancing() )
															{
																int instance_Count = mesh->InstanceCount();
																ImGui::SameLine(); ImGui::TextColored( ImVec4( 0.84f, 0.59f, 0.45f, 1.0f ),
																									   "(Instance Count: %d)", instance_Count );
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
							ImGui::TextUnformatted( pass.target_framebuffer->name.c_str() );

							if( not pass_has_content_to_render )
								ImGuiUtility::EndDisabledButInteractable();
						}

						ImGui::EndTable();
					}
			
					/* ------------------------------- */

					// TODO: Replace with actual FS effect icon.

					ImGui::SeparatorText( ICON_FA_QUESTION " Fullscreen Effects" );

					if( ImGui::BeginTable( "Render Pipeline", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_PreciseWidths ) )
					{
						ImGui::TableSetupColumn( "Name" );
						ImGui::TableSetupColumn( ICON_FA_OBJECT_GROUP " Target Framebuffer" );

						ImGui::TableHeadersRow();
						ImGui::TableNextRow();

						auto DrawEffect = [ &style ]( FullscreenEffect* effect, bool is_always_enabled = false )
						{
							ImGui::TableNextColumn();

							ImGui::PushID( effect->name.c_str() );

							if( not is_always_enabled )
								ImGuiUtility::EyeCheckbox( "", &effect->is_enabled );
							else
							{
								// Reserve the exact same horizontal space as an eye checkbox:
								ImGui::Dummy( ImVec2( ImGui::GetFrameHeight() + style.ItemInnerSpacing.x,
													  ImGui::GetFrameHeight() ) );
							}

							if( not effect->is_enabled )
								ImGuiUtility::BeginDisabledButInteractable();

							ImGui::PopID();
							ImGui::SameLine();

							if( ImGui::TreeNodeEx( effect->name.c_str(), 0, ICON_FA_QUESTION " %s", effect->name.c_str() ) )
							{
								ImGui::TableNextColumn();

								for( std::uint8_t i = 0; i < effect->steps.size(); i++ )
								{
									ImGui::TableNextColumn();
									ImGui::Text( "Step %d", i );

									ImGui::TableNextColumn();
									const auto& step = effect->steps[ i ];
									ImGui::TextUnformatted( step.framebuffer_target->name.c_str() );
								}

								ImGui::TreePop();
							}
							else
								ImGui::TableNextColumn();

							if( not effect->is_enabled )
								ImGuiUtility::EndDisabledButInteractable();
						};

						if( main_framebuffer.SampleCount() > 1 )
							DrawEffect( introspection_surface.msaa_resolve );

						for( auto& [ effect_name, effect ] : *introspection_surface.post_processing_effect_map )
							DrawEffect( effect );

						DrawEffect( introspection_surface.tone_mapping, true );

						ImGui::EndTable();
					}

					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( ICON_FA_OBJECT_GROUP " Framebuffers" ) )
				{
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
							if( ImGui::TreeNodeEx( framebuffer.name.c_str()) )
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

					for( auto& framebuffer : renderer.Framebuffers() )
						DrawFramebufferImGui( framebuffer );

					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Shadow Mapping" ) )
				{
					auto& shadow_mapping_projection_parameters = *introspection_surface.shadow_mapping_projection_parameters;

					ImGui::PushItemWidth( ImGui::CalcTextSize( "999.9" ).x + style.FramePadding.x * 2 );

				/* Row 1: */
					const float button_width( ImGui::CalcTextSize( "XXXXXX" ).x + style.ItemInnerSpacing.x );
					ImGui::Dummy( ImVec2{ 2.0f * button_width, 0 } );
					ImGui::SameLine();
					ImGui::InputFloat( " Top", &shadow_mapping_projection_parameters.top, 0.0f, 0.0f, "%.1f" );
				/* Row 2: */
					ImGui::Dummy( ImVec2{ 3.0f * button_width, 0 } );
					ImGui::SameLine();
					ImGui::InputFloat( " Far", &shadow_mapping_projection_parameters.far, 0.0f, 0.0f, "%.1f" );
				/* Row 3: */
					ImGui::InputFloat( " Left", &shadow_mapping_projection_parameters.left, 0.0f, 0.0f, "%.1f" );
					ImGui::SameLine();
					ImGui::Dummy( ImVec2{ 2.0f * button_width, 0 } );
					ImGui::SameLine();
					ImGui::InputFloat( " Right", &shadow_mapping_projection_parameters.right, 0.0f, 0.0f, "%.1f" );
				/* Row 4: */
					ImGui::Dummy( ImVec2{ button_width, 0 } );
					ImGui::SameLine();
					ImGui::InputFloat( " Near", &shadow_mapping_projection_parameters.near, 0.0f, 0.0f, "%.1f" );
				/* Row 5: */
					ImGui::Dummy( ImVec2{ 2.0f * button_width, 0 } );
					ImGui::SameLine();
					ImGui::InputFloat( " Bottom", &shadow_mapping_projection_parameters.bottom, 0.0f, 0.0f, "%.1f" );

					ImGui::PopItemWidth();

					ImGui::EndTabItem();
				}

				if( ImGui::BeginTabItem( "Other" ) )
				{
					/* MSAA Setting: */
					{
						const auto& msaa_supported_sample_counts = renderer.MSAASupportedSampleCountsFor( main_framebuffer.color_attachment.PixelFormat() );
						const int   option_count = ( int )msaa_supported_sample_counts.size();

						int msaa_sample_log_2 = Math::Log2( main_framebuffer.SampleCount() ); // Can be directly used as index.

						const auto sample_count_string = msaa_sample_log_2 == 0
							? "Off"
							: "MSAA " + std::to_string( msaa_supported_sample_counts[ msaa_sample_log_2 ] ) + 'x';
						if( ImGui::SliderInt( "MSAA", &msaa_sample_log_2, 0, ( int )option_count - 1, sample_count_string.c_str() ) )
						{
							renderer.SetMSAASampleCount( Math::Pow2( msaa_sample_log_2 ) );
						}
					}

					/* Wireframe: */
					ImGui::NewLine();
					ImGui::SeparatorText( "Wireframe Settings" );
					{
						ImGui::SliderFloat( "Wireframe Thickness", &renderer.wireframe_thickness_in_pixels, 0.0f, 100.0f, "%.1f pixels", ImGuiSliderFlags_Logarithmic );
						ImGuiDrawer::Draw( renderer.wireframe_color, "Wireframe Color" );
					}

					/* Bloom: */
					ImGui::NewLine();
					ImGui::SeparatorText( "Bloom" );
					{
						/* Step count: */
						int bloom_step_count = ( int )renderer.GetBloomStepCount();
						if( ImGui::InputInt( "Step Count", &bloom_step_count, 1, 0 ) )
							renderer.SetBloomStepCount( bloom_step_count );

						/* Anti-flicker: */
						int anti_flicker_option = renderer.GetBloomAntiFlickerSetting();
						const char* option_names[ 3 ] = { "Off", "Coarse", "Fine" };
						if( ImGui::SliderInt( "Anti-flicker (Firefly Mitigation)", &anti_flicker_option, 0, 2, option_names[ anti_flicker_option ] ) )
							renderer.SetBloomAntiFlickerSetting( ( Renderer::BloomAntiFlickerSetting )anti_flicker_option );
					}

					/* Misc.: */
					ImGui::NewLine();
					ImGui::SeparatorText( "Misc." );
					{
						local_persist bool is_running = false;
						ImGui::BeginDisabled( is_running );
						if( ImGui::Button( "Flash Main Framebuffer Clear Color" ) )
							main_framebuffer.Debug_FlashClearColor( is_running = true );

						if( ImGuiDrawer::Draw( main_framebuffer.clear_color, "Main Framebuffer Clear Color" ) )
							main_framebuffer.SetClearColor( main_framebuffer.clear_color );
						ImGui::EndDisabled();
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}

		ImGui::End();

		/* Shaders: */
		for( auto& shader : renderer.RegisteredShaders() )
			ImGuiDrawer::Draw( *shader );

		/* Materials: */
		ImGuiDrawer::Draw( *introspection_surface.skybox_material, renderer );
		ImGuiDrawer::Draw( introspection_surface.msaa_resolve->material, renderer );
		for( auto& [ effect_name, effect ] : *introspection_surface.post_processing_effect_map )
			ImGuiDrawer::Draw( effect->material, renderer );
		ImGuiDrawer::Draw( introspection_surface.tone_mapping->material, renderer );

		/* Uniforms Buffers: */
		ImGuiDrawer::Draw( *introspection_surface.uniform_buffer_management_intrinsic, "Shader Intrinsics" );
		ImGuiDrawer::Draw( *introspection_surface.uniform_buffer_management_global,	   "Shader Globals" );
	}
}
