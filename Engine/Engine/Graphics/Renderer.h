#pragma once

// Engine Includes.
#include "Renderable.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Core/BitFlags.hpp"
#include "Core/DirtyBlob.h"
#include "Scene/Camera.h"
#include "Lighting/DirectionalLight.h"
#include "Lighting/PointLight.h"
#include "Lighting/SpotLight.h"
#include "UniformBufferManagement.hpp"

// std Includes.
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace Engine
{
	class Renderer
	{
	public:
		enum class IntrinsicModifyTarget : std::uint8_t
		{
			None = 0,

			UniformBuffer_View                   = 1,
			UniformBuffer_Projection             = 2,
			UniformBuffer_Lighting               = 4,
			UniformBuffer_Lighting_ShadowMapping = 8,

			All = UniformBuffer_View | UniformBuffer_Projection | UniformBuffer_Lighting | UniformBuffer_Lighting_ShadowMapping,
		};

	public:
		static constexpr std::size_t FRAMEBUFFER_OFFSCREEN_COUNT = 2;
	
	public:
		Renderer( const bool enable_gamma_correction,
				  std::array< std::optional< int >, FRAMEBUFFER_OFFSCREEN_COUNT >&& offscreen_framebuffer_msaa_sample_count_values = {},
				  std::array< Texture::Format,		FRAMEBUFFER_OFFSCREEN_COUNT >&& offscreen_framebuffer_color_formats = {} );

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( Renderer );

		~Renderer();

		/* 
		 * Main:
		 */

		void Update();
		void UpdatePerPass( const RenderPass::ID pass_id_to_update, Camera& camera );
		void Render();
		void RenderImGui();
		void OnFramebufferResize( const int new_width_in_pixels, const int new_height_in_pixels );

		/* 
		 * Pass, Queue & Renderable:
		 */

		RenderState& GetRenderState( const RenderPass::ID pass_id_to_fetch );
		void AddPass( const RenderPass::ID new_pass_id, RenderPass&& new_pass );
		void RemovePass( const RenderPass::ID pass_id_to_remove );
		void TogglePass( const RenderPass::ID pass_id_to_toggle, const bool enable );
		void AddQueue( const RenderQueue::ID new_queue_id, RenderQueue&& new_queue );
		void RemoveQueue( const RenderQueue::ID queue_id_to_remove );
		void AddQueueToPass( const RenderQueue::ID queue_id_to_add, const RenderPass::ID pass_to_add_to );
		void RemoveQueueFromPass( const RenderQueue::ID queue_id_to_remove, const RenderPass::ID pass_to_remove_from );
		void ToggleQueue( const RenderQueue::ID queue_id_to_toggle, const bool enable );
		void SetFinalPassToUseEditorFramebuffer();
		void SetFinalPassToUseDefaultFramebuffer();

		void AddRenderable( Renderable* renderable_to_add, const RenderQueue::ID queue_id = RenderQueue::ID{ 0 } );
		// TODO: Switch to unsigned map of "Component" UUIDs when Component class is implemented.
		void RemoveRenderable( Renderable* renderable_to_remove );

		void OnShaderReassign( Shader* previous_shader, const std::string& name_of_material_whose_shader_changed );

		/* 
		 * Lights:
		 */

		void AddDirectionalLight( DirectionalLight* light_to_add );
		void RemoveDirectionalLight();
		/* Only 1 Directional Light can be active at a time! */

		void AddPointLight( PointLight* light_to_add );
		void RemovePointLight( PointLight* light_to_remove );
		void RemoveAllPointLights();

		void AddSpotLight( SpotLight* light_to_add );
		void RemoveSpotLight( SpotLight* light_to_remove );
		void RemoveAllSpotLights();

		/*
		 * Shadow-mapping:
		 */

		inline const Texture* ShadowMapTexture() const { return &light_directional_shadow_map_framebuffer.DepthAttachment(); }

		/*
		 * Shaders:
		 */

		const void* GetShaderGlobal( const std::string& buffer_name ) const;
			  void* GetShaderGlobal( const std::string& buffer_name );

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, value );
		}

		/* For PARTIAL setting of ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_array_instance_name, array_index, value );
		}

		/* For PARTIAL setting of STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_struct_instance_name, value );
		}
		
		/* For PARTIAL setting of NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType >
		void SetShaderGlobal( const std::string& buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			uniform_buffer_management_global.Set( buffer_name, uniform_member_name, value );
		}

		inline const std::unordered_set< Shader* > RegisteredShaders() const { return shaders_registered; }
		void RegisterShader( Shader& shader );
		void UnregisterShader( Shader& shader );

		/*
		 * Stencil Test:
		 */

		void EnableStencilTest();
		void DisableStencilTest();
		void SetStencilWriteMask( const unsigned int mask );
		void SetStencilTestResponses( const StencilTestResponse stencil_fail, const StencilTestResponse stencil_pass_depth_fail, const StencilTestResponse both_pass );
		void SetStencilComparisonFunction( const ComparisonFunction comparison_function, const int reference_value, const unsigned int mask );

		/*
		 * Depth Test:
		 */

		void EnableDepthTest();
		void DisableDepthTest();
		void ToggleDepthWrite( const bool enable );
		void SetDepthComparisonFunction( const ComparisonFunction comparison_function );

		/*
		 * Blending:
		 */

		void EnableBlending();
		void DisableBlending();
		void SetBlendingFactors( const BlendingFactor source_color_factor, const BlendingFactor destination_color_factor,
								 const BlendingFactor source_alpha_factor, const BlendingFactor destination_alpha_factor );
		void SetBlendingFunction( const BlendingFunction function );

		/*
		 * Framebuffer:
		 */

		// TODO: Make these private after some time if they are not used (as pass API makes explicit Framebuffer operations redundant).

		void SetCurrentFramebuffer( Framebuffer* framebuffer );
		void ResetToDefaultFramebuffer( const Framebuffer::BindPoint bind_point = Framebuffer::BindPoint::Both );
		bool DefaultFramebufferIsBound() const;
		Framebuffer* CurrentFramebuffer();
		Framebuffer& EditorFramebuffer();
		Framebuffer& OffscreenFramebuffer( const unsigned int framebuffer_index = 0 );

		/* Color Space: */
		void EnableFramebuffer_sRGBEncoding();
		void DisableFramebuffer_sRGBEncoding();

		/*
		 * Other:
		 */

		void SetPolygonMode( const PolygonMode mode );

	private:

		/*
		 * Main:
		 */

		void InitializeBuiltinQueues();
		void InitializeBuiltinPasses();

		void Render( const Mesh& mesh );
		void Render_Indexed( const Mesh& mesh );
		void Render_NonIndexed( const Mesh& mesh );

		void RenderInstanced( const Mesh& mesh );
		void RenderInstanced_Indexed( const Mesh& mesh );
		void RenderInstanced_NonIndexed( const Mesh& mesh );
	
		void SetIntrinsicsPerPass( const RenderPass& pass );
		void SetIntrinsics( const BitFlags< IntrinsicModifyTarget > targets = IntrinsicModifyTarget::None );

		void UploadIntrinsics();
		void UploadGlobals();

		void CalculateShadowMappingInformation();
		void RecompileModifiedShaders();

		/*
		 * Pass, Queue & Renderable:
		 */

		std::vector< RenderQueue >& RenderQueuesContaining( const Renderable* renderable_of_interest );
		void SetRenderState( const RenderState& render_state_to_set, Framebuffer* target_framebuffer, const bool clear_framebuffer = false );
		void SortRenderablesInQueue( const Vector3& camera_position, std::vector< Renderable* >& renderable_array_to_sort, const SortingMode sorting_mode );

		/*
		 * Face Culling:
		 */

		void EnableFaceCulling();
		void DisableFaceCulling();
		void SetCullFace( const Face face );
		void SetFrontFaceConvention( const WindingOrder winding_order_of_front_faces );

	public:
		
		/* Built-in Pass IDs: */

		static constexpr RenderPass::ID PASS_ID_SHADOW_MAPPING = RenderPass::ID( 10u );
		static constexpr RenderPass::ID PASS_ID_LIGHTING       = RenderPass::ID( 50u );
		static constexpr RenderPass::ID PASS_ID_OUTLINE		   = RenderPass::ID( 100u );
		static constexpr RenderPass::ID PASS_ID_POSTPROCESSING = RenderPass::ID( 220u );

		/* Built-in Queue IDs: */

		/* Using Unity defaults: Background is 1000, Geometry is 2000, AlphaTest is 2450, Transparent is 3000 and Overlay is 4000 */

		static constexpr RenderQueue::ID QUEUE_ID_GEOMETRY          = RenderQueue::ID( 2000u );
		static constexpr RenderQueue::ID QUEUE_ID_GEOMETRY_OUTLINED = RenderQueue::ID( 2350u );
		static constexpr RenderQueue::ID QUEUE_ID_TRANSPARENT       = RenderQueue::ID( 2450u );
		static constexpr RenderQueue::ID QUEUE_ID_SKYBOX            = RenderQueue::ID( 2900u );
		static constexpr RenderQueue::ID QUEUE_ID_POSTPROCESSING    = RenderQueue::ID( 3000u );

	private:

		struct CameraInfo
		{
			Matrix4x4 view_matrix;
			Matrix4x4 projection_matrix;
			Matrix4x4 view_projection_matrix;

			float plane_near;
			float plane_far;
			float aspect_ratio;
			Radians vertical_field_of_view;
		};

	private:

		/*
		 * Logging:
		 */

		GLLogger& logger;

		/*
		 * Framebuffer:
		 */

		Framebuffer* framebuffer_current;
		Framebuffer editor_framebuffer;
		Framebuffer light_directional_shadow_map_framebuffer;
		/* Used for lighting etc. Blitted onto the default framebuffer (or the offscreen editor framebuffer when running in editor). */
		std::array< Framebuffer,			FRAMEBUFFER_OFFSCREEN_COUNT > offscreen_framebuffer_array;
		std::array< std::optional< int >,	FRAMEBUFFER_OFFSCREEN_COUNT > offscreen_framebuffer_msaa_sample_count_array;
		std::array< Texture::Format,		FRAMEBUFFER_OFFSCREEN_COUNT > offscreen_framebuffer_color_format_array;

		/*
		 * Lighting:
		 */

		DirectionalLight*			light_directional;
		std::vector< PointLight* >	lights_point;
		std::vector< SpotLight*	 >	lights_spot;
		int lights_point_active_count;
		int lights_spot_active_count;

		Matrix4x4 light_directional_view_projection_transform_matrix;

		/*
		 * Rendering:
		 */

		std::map< RenderPass::ID,  RenderPass  > render_pass_map;
		std::map< RenderQueue::ID, RenderQueue > render_queue_map;

		std::unordered_set< Shader* > shaders_registered;
		std::unordered_map< Shader*, Shader::ReferenceCount > shaders_registered_reference_count_map;

		/*
		 * Uniform Management:
		 */

		std::unordered_set< const Shader* > shaders_using_intrinsics_lighting;
		std::unordered_set< const Shader* > shaders_using_intrinsics_other;

		UniformBufferManagement< DirtyBlob > uniform_buffer_management_global;
		UniformBufferManagement< DirtyBlob > uniform_buffer_management_intrinsic;

		CameraInfo current_camera_info;

		bool update_uniform_buffer_lighting;
		bool update_uniform_buffer_other;

		/*
		 * Color Space:
		 */

		bool framebuffer_sRGB_encoding_is_enabled;
		bool gamma_correction_is_enabled;
		
		/* 4 bytes of padding. */
	};
}
