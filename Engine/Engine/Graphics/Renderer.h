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
#include "Math/OrthographicProjectionParameters.h"

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

		struct Description
		{
			bool enable_gamma_correction                    = true;
			Texture::Format main_framebuffer_color_format   = Texture::Format::RGBA_16F;
			std::uint8_t main_framebuffer_msaa_sample_count = 4;
			std::initializer_list< Framebuffer::Description > custom_framebuffer_descriptions;
		};

	public:
		static constexpr std::size_t FRAMEBUFFER_CUSTOM_AVAILABLE_COUNT = 4;
	
	public:
		Renderer( Description&& );

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

#ifdef _EDITOR
		/*
		 * Editor:
		 */
		inline EditorShadingMode GetEditorShadingMode() const { return editor_shading_mode; };
		void SetEditorShadingMode( const EditorShadingMode new_editor_shading_mode );
#endif // _EDITOR

		/* 
		 * Pass, Queue & Renderable:
		 */

		RenderState& GetRenderState( const RenderPass::ID pass_id_to_fetch );

		void AddPass( const RenderPass::ID new_pass_id, RenderPass&& new_pass );
		void RemovePass( const RenderPass::ID pass_id_to_remove );
		void TogglePass( const RenderPass::ID pass_id_to_toggle, const bool enable );

		bool PassHasContentToRender( const RenderPass& pass_to_query ) const;

		void AddQueue( const RenderQueue::ID new_queue_id, RenderQueue&& new_queue );
		void RemoveQueue( const RenderQueue::ID queue_id_to_remove );
		void ToggleQueue( const RenderQueue::ID queue_id_to_toggle, const bool enable );

		bool QueueHasContentToRender( const RenderQueue& queue_to_query ) const;

		void AddQueueToPass( const RenderQueue::ID queue_id_to_add, const RenderPass::ID pass_to_add_to );
		void RemoveQueueFromPass( const RenderQueue::ID queue_id_to_remove, const RenderPass::ID pass_to_remove_from );
		
		void SetFinalPassToUseFinalFramebuffer();
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

		inline const Texture* ShadowMapTexture() const { return &framebuffer_shadow_map_light_directional.DepthAttachment(); }

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
		Framebuffer& MainFramebuffer();
		Framebuffer& PostProcessingFramebuffer_A();
		Framebuffer& PostProcessingFramebuffer_B();
		Framebuffer& FinalFramebuffer();
		Framebuffer& CustomFramebuffer( const unsigned int framebuffer_index = 0 );

		/*
		 * Color Space:
		 */
		void EnableFramebuffer_sRGBEncoding();
		void DisableFramebuffer_sRGBEncoding();

		/*
		 * Queries:
		 */

		/* Logs a warning if the sample count in question is not available for the given format. */
		static bool CheckMSAASupport( const Texture::Format format, const std::uint8_t sample_count_to_query );

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

		void InitializeInternalMeshes();
		void InitializeInternalShaders();
		void InitializeInternalMaterials();
		void InitializeInternalRenderables();

		void SetPolygonMode( const PolygonMode mode );

		/*
		 * Pass, Queue & Renderable:
		 */

		std::vector< RenderQueue* >& RenderQueuesContaining( const Renderable* renderable_of_interest );
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
		static constexpr RenderPass::ID PASS_ID_MSAA_RESOLVE   = RenderPass::ID( 220u );
		static constexpr RenderPass::ID PASS_ID_POSTPROCESSING = RenderPass::ID( ( std::uint8_t )PASS_ID_MSAA_RESOLVE + 1u );
		static constexpr RenderPass::ID PASS_ID_FINAL          = RenderPass::ID( std::numeric_limits< std::uint8_t >::max() );

		static constexpr std::array< RenderPass::ID, 5 > BUILTIN_PASS_ID_LIST =
		{
			PASS_ID_SHADOW_MAPPING,
			PASS_ID_LIGHTING,
			PASS_ID_MSAA_RESOLVE,
			PASS_ID_POSTPROCESSING,
			PASS_ID_FINAL
		};

		/* Built-in Queue IDs: */

		/* Using Unity defaults: Background is 1000, Geometry is 2000, AlphaTest is 2450, Transparent is 3000 and Overlay is 4000 */

		static constexpr RenderQueue::ID QUEUE_ID_GEOMETRY                       = RenderQueue::ID(  2'000u );
		static constexpr RenderQueue::ID QUEUE_ID_TRANSPARENT                    = RenderQueue::ID(  2'450u );
		static constexpr RenderQueue::ID QUEUE_ID_SKYBOX                         = RenderQueue::ID(  2'900u );
		static constexpr RenderQueue::ID QUEUE_ID_MSAA_RESOLVE                   = RenderQueue::ID(  3'000u );

		static constexpr RenderQueue::ID QUEUE_ID_BEFORE_POSTPROCESSING          = RenderQueue::ID( 3'001u );
		/* These are multi-step post-processing effects where each effect reads the previous effect's result and writes to a different framebuffer (ping-ponging). */
		static constexpr RenderQueue::ID QUEUE_ID_POSTPROCESSING_BLOOM           = RenderQueue::ID( 3'002u );

		static constexpr RenderQueue::ID QUEUE_ID_FINAL	                         = RenderQueue::ID( 5'000u );

		static constexpr std::array< RenderQueue::ID, 7 > BUILTIN_QUEUE_ID_LIST =
		{
			QUEUE_ID_GEOMETRY,
			QUEUE_ID_TRANSPARENT,
			QUEUE_ID_SKYBOX,
			QUEUE_ID_MSAA_RESOLVE,
			QUEUE_ID_BEFORE_POSTPROCESSING,
			QUEUE_ID_POSTPROCESSING_BLOOM,
			QUEUE_ID_FINAL
		};

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

		Framebuffer framebuffer_default;
		Framebuffer* framebuffer_current;

		Framebuffer framebuffer_shadow_map_light_directional;
		Framebuffer framebuffer_main;
		Framebuffer framebuffer_postprocessing_A; // Used in ping-pong fashion.
		Framebuffer framebuffer_postprocessing_B; // Used in ping-pong fashion.
		Framebuffer framebuffer_final;

		std::array< Framebuffer,								FRAMEBUFFER_CUSTOM_AVAILABLE_COUNT > framebuffer_custom_array;
		std::array< std::optional< Framebuffer::Description >,	FRAMEBUFFER_CUSTOM_AVAILABLE_COUNT > framebuffer_custom_description_array;

		std::uint8_t framebuffer_main_msaa_sample_count;
		Texture::Format framebuffer_main_color_format;

		/* 4 bytes of padding. */

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

		Engine::Mesh full_screen_quad_mesh;

		/*
		 * Rendering/MSAA:
		 */

		Engine::Material msaa_resolve_material;
		Engine::Renderable msaa_resolve_renderable;
		Engine::Shader* shader_msaa_resolve;

		/*
		 * Rendering/Tone Mapping:
		 */

		Engine::Material tone_mapping_material;
		Engine::Renderable tone_mapping_renderable;
		Engine::Shader* shader_tone_mapping;

		/*
		 * Shadow Mapping:
		 */

		OrthographicProjectionParameters shadow_mapping_projection_parameters;

		/*
		 * Uniform Management:
		 */

		std::unordered_set< const Shader* > shaders_using_intrinsics_lighting;
		std::unordered_set< const Shader* > shaders_using_intrinsics_other;

		UniformBufferManagement< DirtyBlob > uniform_buffer_management_global;
		UniformBufferManagement< DirtyBlob > uniform_buffer_management_intrinsic;

		CameraInfo current_camera_info;

		bool shaders_need_uniform_buffer_lighting;
		bool shaders_need_uniform_buffer_other;

		/*
		 * Color Space:
		 */

		bool framebuffer_sRGB_encoding_is_enabled;
		bool gamma_correction_is_enabled;

		/*
		 * Editor:
		 */

		EditorShadingMode editor_shading_mode;
		
		/* 4 bytes of padding. */
	};
}
