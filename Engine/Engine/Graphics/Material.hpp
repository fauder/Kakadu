#pragma once

// Engine Includes.
#include "Buffer.hpp"
#include "Shader.hpp"
#include "UniformBufferManagement.hpp"
#include "Texture.h"
#include "Core/Blob.hpp"

// std Includes.
#include <cstddef> // std::byte.
#include <unordered_map>
#include <vector>

namespace Engine
{
	class Renderer;

	class Material
	{
		friend Renderer;
		 
	public:
		Material();
		Material( const std::string& name );
		Material( const std::string& name, Shader* const shader );

		DELETE_COPY_CONSTRUCTORS( Material );
		DEFAULT_MOVE_CONSTRUCTORS( Material );

		~Material();

	/* Queries: */
		const std::string& Name() const { return name; }

	/* Main: */
		const Shader* Bind() const;

	/* Shader: */
		const std::string& GetShaderName() const
		{
			ASSERT_DEBUG_ONLY( HasShaderAssigned() && "Material::GetShaderName() called with nullptr shader!" );
			return shader->Name();
		}
		bool HasShaderAssigned() const { return shader; }

		Shader* GetShader();
		void SetShader( Shader* shader );

	/* Uniforms: */
		const std::unordered_map< std::string, Uniform::Information				>& GetUniformInfoMap()			const { return *uniform_info_map; }
		const std::unordered_map< std::string, const Uniform::BufferInformation	>& GetUniformBufferInfoMap()	const { return uniform_buffer_management_regular
																																	.GetBufferInformationMap();	}
		const std::unordered_map< std::string, const Texture*					>& GetTextureMap()				const { return texture_map; }

		bool HasUniform( const std::string& uniform_name ) const { return uniform_info_map->contains( uniform_name ); }
		bool HasUniformBuffer( const std::string& uniform_buffer_name ) const { return uniform_buffer_management_regular.GetBufferInformationMap().contains( uniform_buffer_name ); }

		const void* Get( const Uniform::Information& uniform_info ) const;
			  void* Get( const Uniform::Information& uniform_info );
		const void* Get( const std::string& uniform_buffer_name ) const;
			  void* Get( const std::string& uniform_buffer_name );
		
		template< typename UniformType >
		void Set( const std::string& uniform_name, const UniformType& value )
		{
#ifdef _EDITOR
			if( not HasUniform( uniform_name ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
				return;
			}
#endif // _EDITOR

			const auto& uniform_info = GetUniformInformation( uniform_name );

#ifdef _EDITOR
			if( uniform_info.size != sizeof( value ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform ")" + std::string( uniform_name ) + R"(" is being set with a type of unmatching size!)" );
				return;
			}
#endif // _EDITOR

			uniform_blob_default_block.Set( value, uniform_info.offset );
		}

		template< typename UniformType >
		void SetArray( const std::string& uniform_name, const UniformType* address )
		{
#ifdef _EDITOR
			if( not HasUniform( uniform_name ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform (array) ")" + std::string( uniform_name ) + R"(" does not exist!)" );
				return;
			}
#endif // _EDITOR

			const auto& uniform_info = GetUniformInformation( uniform_name );

#ifdef _EDITOR
			if( uniform_info.size != sizeof( UniformType ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform (array) ")" + std::string( uniform_name ) + R"(" is being set with a pointer type of unmatching size!)" );
				return;
			}
#endif // _EDITOR

			uniform_blob_default_block.Set( ( std::byte* )address, uniform_info.offset, sizeof( UniformType )* uniform_info.count_array );
		}

		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void Set( const std::string& uniform_buffer_name, const StructType& value )
		{
			uniform_buffer_management_regular.Set( uniform_buffer_name, value );
		}

		/* For PARTIAL setting ARRAY uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetPartial_Array( const std::string& uniform_buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const StructType& value )
		{
			uniform_buffer_management_regular.SetPartial_Array( uniform_buffer_name, uniform_member_array_instance_name, array_index, value );
		}

		/* For PARTIAL setting ARRAY uniforms INSIDE a Uniform Buffer. */
		void SetPartial_Array( const std::string& uniform_buffer_name, const char* uniform_member_array_instance_name, const unsigned int array_index, const std::byte* value )
		{
			uniform_buffer_management_regular.SetPartial_Array( uniform_buffer_name, uniform_member_array_instance_name, array_index, value );
		}

		/* For PARTIAL setting STRUCT uniforms INSIDE a Uniform Buffer. */
		template< typename StructType > requires( std::is_base_of_v< Std140StructTag, StructType > )
		void SetPartial_Struct( const std::string& uniform_buffer_name, const char* uniform_member_struct_instance_name, const StructType& value )
		{
			uniform_buffer_management_regular.SetPartial_Struct( uniform_buffer_name, uniform_member_struct_instance_name, value );
		}

		/* For PARTIAL setting STRUCT uniforms INSIDE a Uniform Buffer. */
		void SetPartial_Struct( const std::string& uniform_buffer_name, const char* uniform_member_struct_instance_name, const std::byte* value )
		{
			uniform_buffer_management_regular.SetPartial_Struct( uniform_buffer_name, uniform_member_struct_instance_name, value );
		}

		/* For PARTIAL setting NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		template< typename UniformType > requires( not std::is_pointer_v< UniformType > ) // Don't want to "override" the overload below for actual pointer types.
		void SetPartial( const std::string& uniform_buffer_name, const char* uniform_member_name, const UniformType& value )
		{
			uniform_buffer_management_regular.SetPartial( uniform_buffer_name, uniform_member_name, value );
		}

		/* For PARTIAL setting NON-AGGREGATE uniforms INSIDE a Uniform Buffer. */
		void SetPartial( const std::string& uniform_buffer_name, const char* uniform_member_name, const std::byte* value )
		{
			uniform_buffer_management_regular.SetPartial( uniform_buffer_name, uniform_member_name, value );
		}

	/* Textures: */
		void SetTexture( const char* sampler_name_of_new_texture, const Texture* texture_to_be_set );
		const Texture* GetTexture( const char* sampler_name_of_new_texture ) const;

	private:
	/* Shader: */
		const Shader* GetShader() const { return shader; };

	/* Uniform: */
		const Uniform::Information& GetUniformInformation( const std::string& uniform_name ) const;

		void UploadUniform( const Uniform::Information& uniform_info );
		void UploadUniforms(); // Renderer calls this, it has private access through friend declaration.

		template< typename UniformType >
		void SetAndUploadUniform( const std::string& uniform_name, const UniformType& value ) // Renderer calls this, it has private access through friend declaration.
		{
#ifdef _EDITOR
			if( not HasUniform( uniform_name ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform ")" + std::string( uniform_name ) + R"(" does not exist!)" );
				return;
			}
#endif // _EDITOR

			Set( uniform_name, value );
			const auto& uniform_info = GetUniformInformation( uniform_name );

#ifdef _EDITOR
			if( uniform_info.size != sizeof( value ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform ")" + std::string( uniform_name ) + R"(" is being set & uploaded with a type of unmatching size!)" );
				return;
			}
#endif // _EDITOR

			uniform_blob_default_block.Set( value, uniform_info.offset );

			UploadUniform( uniform_info );
		}

		template< typename UniformType >
		void SetAndUploadUniformArray( const std::string& uniform_name, const UniformType& value ) // Renderer calls this, it has private access through friend declaration.
		{
#ifdef _EDITOR
			if( not HasUniform( uniform_name ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform (array) ")" + std::string( uniform_name ) + R"(" does not exist!)" );
				return;
			}
#endif // _EDITOR

			Set( uniform_name, value );
			const auto& uniform_info = GetUniformInformation( uniform_name );

#ifdef _EDITOR
			if( uniform_info.size != sizeof( UniformType ) )
			{
				ServiceLocator< GLLogger >::Get().Error( R"(Material ")" + name + R"(": uniform (array) ")" + std::string( uniform_name ) + R"(" is being set & uploaded with a pointer type of unmatching size!)" );
				return;
			}
#endif // _EDITOR

			uniform_blob_default_block.Set( value, uniform_info.offset, uniform_info.count_array );

			UploadUniform( uniform_info );
		}

	/* Texture: */
		void PopulateTextureMap();
		void RepopulateTextureMap();

	private:
		std::string name;

		Shader* shader;

		/* Allocated/Resized only when a shader is assigned. 
		 * NOTE: Can't use a DirtyBlob for the default block as each Material possibly has different values per Material, thus the uniforms need to be updated per-frame. */
		Blob uniform_blob_default_block;

		/* Map pointer below is assigned only when the Shader itself is assigned/re-assigned to the Material, through Shader::GetUniformInfoMap(). */
		const std::unordered_map< std::string, Uniform::Information >* uniform_info_map;

		UniformBufferManagement< Blob > uniform_buffer_management_regular; // Read above for why this does not use a DirtyBlob instead of a regular Blob.

		std::unordered_map< std::string, const Texture* > texture_map;
	};
}
