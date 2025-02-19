#pragma once

// Engine Includes.
#include "Core/BitFlags.hpp"
#include "Graphics/Color.hpp"
#include "Graphics/Lighting/DirectionalLight.h"
#include "Graphics/Lighting/PointLight.h"
#include "Graphics/Lighting/SpotLight.h"
#include "Graphics/Material.hpp"
#include "Graphics/Renderer.h"
#include "Graphics/UniformBufferManagement.hpp"
#include "Math/Polar.h"
#include "Math/Quaternion.hpp"
#include "Scene/Camera.h"

// Vendor Includes.
#include "ImGui/imgui.h"

namespace Engine::ImGuiDrawer
{
	struct State
	{
		bool window_material_padding_hide = true;
	};
	
	void Initialize();
	void Update();

	template< typename Type >
	constexpr ImGuiDataType_ GetImGuiDataType()
	{
		if constexpr( std::is_same_v< Type, int > )
			return ImGuiDataType_S32;
		if constexpr( std::is_same_v< Type, unsigned int > )
			return ImGuiDataType_U32;
		if constexpr( std::is_same_v< Type, float > )
			return ImGuiDataType_Float;
		if constexpr( std::is_same_v< Type, double > )
			return ImGuiDataType_Double;
	}

	template< typename Type >
	constexpr const char* GetFormat()
	{
		if constexpr( std::is_same_v< Type, int > )
			return "%d";
		if constexpr( std::is_same_v< Type, unsigned int > )
			return "%u";
		if constexpr( std::is_same_v< Type, float > )
			return "%.2f";
		if constexpr( std::is_same_v< Type, double > )
			return "%.2lf";
	}

	bool Draw( const GLenum type,	    void* value_pointer, const char* name = "##hidden" );
	void Draw( const GLenum type, const void* value_pointer, const char* name = "##hidden" );

	bool Draw( int& scalar,					const char* name = "##scalar_int" 	 );
	bool Draw( int& scalar,					const int min, const int max, const char* name = "##scalar_int" );
	void Draw( const int& scalar,			const char* name = "##scalar_int"    );
	bool Draw( unsigned int& scalar,		const char* name = "##scalar_uint"	 );
	bool Draw( unsigned int& scalar,		const unsigned int min, const unsigned int max, const char* name = "##scalar_uint" );
	void Draw( const unsigned int& scalar,	const char* name = "##scalar_uint"	 );
	bool Draw( float& scalar,				const char* name = "##scalar_float",  const char* format = GetFormat< float  >() );
	void Draw( const float& scalar,			const char* name = "##scalar_float",  const char* format = GetFormat< float  >() );
	bool Draw( double& scalar,				const char* name = "##scalar_double", const char* format = GetFormat< double >() );
	void Draw( const double& scalar,		const char* name = "##scalar_double", const char* format = GetFormat< double >() );
	bool Draw( bool& value,					const char* name = "##bool" );
	void Draw( const bool& value,			const char* name = "##bool" );

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	void Draw( const Math::Vector< Component, Size >& vector, const char* name = "##vector<>" )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		const auto& style = ImGui::GetStyle();

		if constexpr( std::is_same_v< Component, bool > )
		{
			ImGui::PushItemWidth( Size * ImGui::CalcTextSize( " []" ).x + ( Size - 1 ) * style.ItemInnerSpacing.x );
			if constexpr( Size >= 2 )
			{
				bool x = vector.X(), y = vector.Y();
				ImGui::Checkbox( "##x", &x ); ImGui::SameLine(); ImGui::Checkbox( "##y", &y );
			}
			if constexpr( Size >= 3 )
			{
				bool value = vector.Z();
				ImGui::SameLine(); ImGui::Checkbox( "##z", &value );
			}
			if constexpr( Size >= 4 )
			{
				bool value = vector.W();
				ImGui::SameLine(); ImGui::Checkbox( "##w", &value );
			}
		}
		else
		{
			ImGui::PushItemWidth( Size * ImGui::CalcTextSize( ".-999.99" ).x + ( Size - 1 ) * style.ItemInnerSpacing.x );
			/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
			ImGui::InputScalarN( name, GetImGuiDataType< Component >(), const_cast< Component* >( vector.Data() ), Size, NULL, NULL, GetFormat< Component >(), ImGuiInputTextFlags_ReadOnly );
		}
		ImGui::PopItemWidth();

		ImGui::PopStyleColor();
	}

	template< Concepts::Arithmetic Component, std::size_t Size > requires( Size > 1 )
	bool Draw( Math::Vector< Component, Size >& vector, const char* name = "##vector<>" )
	{
		bool is_modified = false;

		const auto& style = ImGui::GetStyle();

		if constexpr( std::is_same_v< Component, bool > )
		{
			ImGui::PushItemWidth( Size * ImGui::CalcTextSize( " []" ).x + ( Size - 1 ) * style.ItemInnerSpacing.x );
			if constexpr( Size >= 2 )
			{
				is_modified |= ImGui::Checkbox( "##x", &vector[ 0 ] ); ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##y", &vector[ 1 ] );
			}
			if constexpr( Size >= 3 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##z", &vector[ 2 ] );
			}
			if constexpr( Size >= 4 )
			{
				ImGui::SameLine(); is_modified |= ImGui::Checkbox( "##w", &vector[ 3 ] );
			}
		}
		else
		{
			ImGui::PushItemWidth( Size * ImGui::CalcTextSize( ".-999.99" ).x + ( Size - 1 ) * style.ItemInnerSpacing.x );
			is_modified |= ImGui::DragScalarN( name, GetImGuiDataType< Component >(), vector.Data(), Size, 1.0f, NULL, NULL, GetFormat< Component >() );
		}
		ImGui::PopItemWidth();

		return is_modified;
	}

	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize > requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
	void Draw( const Math::Matrix< Type, RowSize, ColumnSize >& matrix, const char* name = "##matrix<>" )
	{
		if( ImGui::TreeNodeEx( name, 0 ) )
		{
			ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

			const auto& style = ImGui::GetStyle();

			ImGui::PushItemWidth( ColumnSize * ImGui::CalcTextSize( ".-999.99" ).x + ( ColumnSize - 1 ) * style.ItemInnerSpacing.x );
			for( auto row_index = 0; row_index < RowSize; row_index++ )
			{
				const auto& row_vector = matrix.GetRow< ColumnSize >( row_index );
				ImGui::PushID( row_index );
				Draw( row_vector );
				ImGui::PopID();
			}
			ImGui::PopItemWidth();

			ImGui::PopStyleColor();

			ImGui::TreePop();
		}
	}

	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize > requires Concepts::NonZero< RowSize >&& Concepts::NonZero< ColumnSize >
	bool Draw( Math::Matrix< Type, RowSize, ColumnSize >& matrix, const char* name = "##matrix<>" )
	{
		bool is_modified = false;

		if( ImGui::TreeNodeEx( name, 0 ) )
		{
			const auto& style = ImGui::GetStyle();

			ImGui::PushItemWidth( ColumnSize * ImGui::CalcTextSize( ".-999.99" ).x + ( ColumnSize - 1 ) * style.ItemInnerSpacing.x );
			auto& first_row_vector = matrix.GetRow< ColumnSize >();
			is_modified |= Draw( first_row_vector );

			if( name[ 0 ] != '#' || name[ 1 ] != '#' )
			{
				ImGui::SameLine( 0.0f, style.ItemInnerSpacing.x );
				ImGui::TextUnformatted( name );
			}

			for( auto row_index = 1; row_index < RowSize; row_index++ )
			{
				ImGui::PushID( row_index );
				auto& row_vector = matrix.GetRow< ColumnSize >( row_index );
				is_modified |= Draw( row_vector );
				ImGui::PopID();
			}

			ImGui::TreePop();
			ImGui::PopItemWidth();
		}

		return is_modified;
	}

	template< Concepts::Arithmetic Component >
	void Draw( const Math::Quaternion< Component >& quaternion, const char* name = "##quaternion<>" )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		Math::Vector< Math::Degrees< Component >, 3 > euler;
		Math::QuaternionToEuler( quaternion, euler );

		const auto& style = ImGui::GetStyle();

		ImGui::PushItemWidth( 3.0f * ImGui::CalcTextSize( ".-999.99" ).x + 2.0f * style.ItemInnerSpacing.x );
		/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::InputScalarN( name, GetImGuiDataType< Component >(), euler.Data(), euler.Dimension(), NULL, NULL, GetFormat< Component >(), ImGuiInputTextFlags_ReadOnly );
		ImGui::PopItemWidth();

		ImGui::PopStyleColor();
	}

	template< Concepts::Arithmetic Component >
	bool Draw( Math::Quaternion< Component >& quaternion, const char* name = "##quaternion<>" )
	{
		Math::Vector< Math::Degrees< Component >, 3 > euler;
		Math::QuaternionToEuler( quaternion, euler );

		const auto& style = ImGui::GetStyle();

		ImGui::PushItemWidth( 3.0f * ImGui::CalcTextSize( ".-999.99" ).x + 2.0f * style.ItemInnerSpacing .x );
		const bool is_modified = ImGui::DragScalarN( name, GetImGuiDataType< Component >(), euler.Data(), euler.Dimension(), 1.0f, NULL, NULL, GetFormat< Component >() );
		if( is_modified )
			quaternion = Math::EulerToQuaternion( euler );
		ImGui::PopItemWidth();

		return is_modified;
	}

	bool Draw(		 Color3& color, const char* name = "##color3" );
	void Draw( const Color3& color, const char* name = "##color3" );
	bool Draw(		 Color4& color, const char* name = "##color4" );
	void Draw( const Color4& color, const char* name = "##color4" );

	template< template< class > class AngleType, typename FloatType >
	bool Draw( AngleType< FloatType >& angle, const char* name,
			   const AngleType< FloatType >& min = AngleType< FloatType >( 0 ), const AngleType< FloatType >& max = Constants< AngleType< FloatType > >::Two_Pi(),
			   const char* format = AngleType< FloatType >::Format( "%.4g" ).data() )
	{
		return ImGui::SliderFloat( name, ( float* )&angle, ( float )min, ( float )max, format );
	}

	template< template< class > class AngleType, typename FloatType >
	void Draw( const AngleType< FloatType >& angle, const char* name = "##angle", const char* format = AngleType< FloatType >::Format( "%.4g" ).data() )
	{
		ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_TextDisabled ) );

		/* Since the read-only flag is passed, the passed pointer will not be modified. So this hack is safe to use here. */
		ImGui::InputFloat( name, const_cast< float* >( &angle.Value() ), 0, 0, format, ImGuiInputTextFlags_ReadOnly );

		ImGui::PopStyleColor();
	}

	bool Draw(		 Math::Polar2& polar_coords, const bool show_radius = true, const char* name = "##polar_coords" );
	void Draw( const Math::Polar2& polar_coords,								const char* name = "##polar_coords" );
	bool Draw(		 Math::Polar3_Spherical_Game& spherical_coords, const bool show_radius = true, const char* name = "##spherical_coords" );
	void Draw( const Math::Polar3_Spherical_Game& spherical_coords,								   const char* name = "##spherical_coords" );

	bool Draw(		 Texture* texture, const char* name = "##texture" );
	void Draw( const Texture* texture, const char* name = "##texture" );

	void Draw( const std::map< std::string, Texture >& texture_map, const Vector2& window_size = { 512.0f, 512.0f } );

	bool Draw(		 Camera& camera, const char* name = "##camera", const bool disable_aspect_ratio_and_fov = false );
	void Draw( const Camera& camera, const char* name = "##camera" );

	bool Draw(		 Transform& transform, const BitFlags< Transform::Mask > = Transform::Mask::All, const char* name = "##transform" );
	void Draw( const Transform& transform, const BitFlags< Transform::Mask > = Transform::Mask::All, const char* name = "##transform" );

	bool Draw( Material& material, Renderer& renderer, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	/* const version of Material is nearly the same as the non-const and it was getting annoying having to update a huge and mostly unused function, so it is removed. */

	template< typename BlobType > requires( std::is_base_of_v< Blob, BlobType > )
	bool Draw( UniformBufferManagement< BlobType >& buffer_management, const char* name = "##buffer-management", ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing )
	{
		if( buffer_management.GetBufferInformationMap().empty() )
			return false;

		bool is_modified = false;

		if( ImGui::Begin( name, nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::BeginTable( name, 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PreciseWidths ) )
			{
				const auto& uniform_buffer_info_map = buffer_management.GetBufferInformationMap();

				ImGui::TableSetupColumn( "Name"	 );
				ImGui::TableSetupColumn( "Value" );

				ImGui::TableHeadersRow();
				ImGui::TableNextRow();

				for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				{
					ImGui::TableNextColumn();

					if( ImGui::TreeNodeEx( uniform_buffer_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
					{
						ImGui::TableNextRow();
						/* No need to update the Material when the Draw() call below returns true; Memory from the blob is provided directly to Draw(), so the Material is updated. */

						std::byte* memory_blob = ( std::byte* )buffer_management.Get( uniform_buffer_name );

						for( const auto& [ uniform_buffer_member_name, uniform_buffer_member_struct_info ] : uniform_buffer_info.members_struct_map )
						{
							ImGui::TableNextColumn();

							if( ImGui::TreeNodeEx( uniform_buffer_member_struct_info.editor_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
							{
								ImGui::TableNextRow();

								for( const auto& uniform_buffer_member_info : uniform_buffer_member_struct_info.members_map )
								{
									ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

									ImGui::TableNextColumn();

									ImGui::PushID( ( void* )uniform_buffer_member_info );
									if constexpr( std::is_same_v< BlobType, DirtyBlob > )
									{
										if( Draw( uniform_buffer_member_info->type, memory_blob + uniform_buffer_member_info->offset ) )
										{
											buffer_management.SetPartial_Struct( uniform_buffer_name, uniform_buffer_member_name.c_str(), 
																				 memory_blob + uniform_buffer_member_info->offset );
											is_modified = true;
										}
									}
									else
										is_modified |= Draw( uniform_buffer_member_info->type, memory_blob + uniform_buffer_member_info->offset );
									ImGui::PopID();
								}

								ImGui::TreePop();
							}
							else
								ImGui::TableNextRow();
						}

						for( const auto& [ uniform_buffer_member_name, uniform_buffer_member_array_info ] : uniform_buffer_info.members_array_map )
						{
							ImGui::TableNextColumn();

							if( ImGui::TreeNodeEx( uniform_buffer_member_array_info.editor_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
							{
								ImGui::TableNextRow();

								for( auto array_index = 0; array_index < uniform_buffer_member_array_info.element_count; array_index++ )
								{
									ImGui::TableNextColumn();

									static char array_member_name_string[ 255 ];
									std::snprintf( array_member_name_string, 255, "%s[%d]", uniform_buffer_member_array_info.editor_name.c_str(), array_index );

									if( ImGui::TreeNodeEx( array_member_name_string, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
									{
										ImGui::TableNextRow();

										for( const auto& uniform_buffer_member_info : uniform_buffer_member_array_info.members_map )
										{
											ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

											ImGui::TableNextColumn();

											std::byte* effective_offset = memory_blob + uniform_buffer_member_info->offset + array_index * uniform_buffer_member_array_info.stride;

											ImGui::PushID( effective_offset );
											if constexpr( std::is_same_v< BlobType, DirtyBlob > )
											{
												if( Draw( uniform_buffer_member_info->type, effective_offset ) )
												{
													buffer_management.SetPartial_Array( uniform_buffer_name, uniform_buffer_member_name.c_str(), 
																						array_index, effective_offset );
													is_modified = true;
												}
											}
											else
												is_modified |= Draw( uniform_buffer_member_info->type, effective_offset );
											ImGui::PopID();
										}

										ImGui::TreePop();
									}
									else
										ImGui::TableNextRow();
								}

								ImGui::TreePop();
							}
							else
								ImGui::TableNextRow();
						}

						for( const auto& [ uniform_buffer_member_name, uniform_buffer_member_single_info ] : uniform_buffer_info.members_single_map )
						{
							ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_single_info->editor_name.c_str() );

							ImGui::TableNextColumn();

							ImGui::PushID( ( void* )uniform_buffer_member_single_info );
							if constexpr( std::is_same_v< BlobType, DirtyBlob > )
							{
								if( Draw( uniform_buffer_member_single_info->type, memory_blob + uniform_buffer_member_single_info->offset ) )
								{
									buffer_management.SetPartial( uniform_buffer_name, uniform_buffer_member_name.c_str(), 
																  memory_blob + uniform_buffer_member_single_info->offset );
									is_modified = true;
								}
							}
							else
								is_modified |= Draw( uniform_buffer_member_single_info->type, memory_blob + uniform_buffer_member_single_info->offset );
							ImGui::PopID();
						}

						ImGui::TreePop();
					}
					else
						ImGui::TableNextRow();
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();

		return is_modified;
	}

	template< typename BlobType > requires( std::is_base_of_v< Blob, BlobType > )
	void Draw( const UniformBufferManagement< BlobType >& buffer_management, const char* name = "##buffer-management",
			   ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing )
	{
		if( ImGui::Begin( name, nullptr, window_flags | ImGuiWindowFlags_AlwaysAutoResize ) )
		{
			if( ImGui::BeginTable( name, 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_PreciseWidths ) )
			{
				const auto& uniform_buffer_info_map = buffer_management.GetBufferInformationMap();

				ImGui::TableSetupColumn( "Name"	 );
				ImGui::TableSetupColumn( "Value" );

				ImGui::TableHeadersRow();
				ImGui::TableNextRow();

				for( const auto& [ uniform_buffer_name, uniform_buffer_info ] : uniform_buffer_info_map )
				{
					ImGui::TableNextColumn();

					if( ImGui::TreeNodeEx( uniform_buffer_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
					{
						ImGui::TableNextRow();
						/* No need to update the Material when the Draw() call below returns true; Memory from the blob is provided directly to Draw(), so the Material is updated. */

						std::byte* memory_blob = ( std::byte* )buffer_management.Get( uniform_buffer_name );

						for( const auto& [ dont_care, uniform_buffer_member_struct_info ] : uniform_buffer_info.members_struct_map )
						{
							ImGui::TableNextColumn();

							if( ImGui::TreeNodeEx( uniform_buffer_member_struct_info.editor_name.c_str(), ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
							{
								ImGui::TableNextRow();

								for( const auto& uniform_buffer_member_info : uniform_buffer_member_struct_info.members_map )
								{
									ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

									ImGui::TableNextColumn();

									ImGui::PushID( ( void* )uniform_buffer_member_info );
									Draw( uniform_buffer_member_info->type, memory_blob + uniform_buffer_member_info->offset );
									ImGui::PopID();
								}

								ImGui::TreePop();
							}
							else
								ImGui::TableNextRow();
						}

						for( const auto& [ dont_care, uniform_buffer_member_array_info ] : uniform_buffer_info.members_array_map )
						{
							ImGui::TableNextColumn();

							if( ImGui::TreeNodeEx( uniform_buffer_member_array_info.editor_name.c_str(), ImGuiTreeNodeFlags_Framed ) )
							{
								ImGui::TableNextRow();

								for( auto array_index = 0; array_index < uniform_buffer_member_array_info.element_count; array_index++ )
								{
									ImGui::TableNextColumn();

									static char array_member_name_string[ 255 ];
									std::snprintf( array_member_name_string, 255, "%s[%d]", uniform_buffer_member_array_info.editor_name.c_str(), array_index );

									if( ImGui::TreeNodeEx( array_member_name_string, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed ) )
									{
										ImGui::TableNextRow();

										for( const auto& uniform_buffer_member_info : uniform_buffer_member_array_info.members_map )
										{
											ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_info->editor_name.c_str() );

											ImGui::TableNextColumn();

											std::byte* effective_offset = memory_blob + uniform_buffer_member_info->offset + array_index * uniform_buffer_member_array_info.stride;

											ImGui::PushID( effective_offset );
											Draw( uniform_buffer_member_info->type, effective_offset );
											ImGui::PopID();
										}

										ImGui::TreePop();
									}
									else
										ImGui::TableNextRow();
								}

								ImGui::TreePop();
							}
							else
								ImGui::TableNextRow();
						}

						for( const auto& [ dont_care, uniform_buffer_member_single_info ] : uniform_buffer_info.members_single_map )
						{
							ImGui::TableNextColumn(); ImGui::TextUnformatted( uniform_buffer_member_single_info->editor_name.c_str() );

							ImGui::TableNextColumn();

							ImGui::PushID( ( void* )uniform_buffer_member_single_info );
							Draw( uniform_buffer_member_single_info->type, memory_blob + uniform_buffer_member_single_info->offset );
							ImGui::PopID();
						}

						ImGui::TreePop();
					}
					else
						ImGui::TableNextRow();
				}

				ImGui::EndTable();
			}
		}

		ImGui::End();
	}

	void Draw( const Shader& shader, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );

	void Draw( const Framebuffer& framebuffer, ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	/* Decorations are icon-like text with rectangles. Example uses are for HDR, MSAA & sRGB. */
	void DrawTextureFormatWithDecorations( const Texture& texture );
	void DrawTextureFormatDecorationsOnly( const Texture& texture );

	bool Draw(		 DirectionalLight&	directional_light,	const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const DirectionalLight&	directional_light,	const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 PointLight&		point_light,		const char* light_name,		const bool hide_position = false,	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const PointLight&		point_light,		const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	bool Draw(		 SpotLight&			spot_light,			const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
	void Draw( const SpotLight&			spot_light,			const char* light_name,											ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoFocusOnAppearing );
}
