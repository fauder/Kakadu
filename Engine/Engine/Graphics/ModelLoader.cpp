// Engine Includes.
#include "Model.h"
#include "Core/AssetDatabase.hpp"
#include "Math/Matrix.h"
#include "Math/Quaternion.hpp"

// Vendor Includes.
#pragma warning(disable:5223)
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#pragma warning(default:5223)

// std Includes.
#include <numeric>

template <>
struct fastgltf::ElementTraits< Engine::Vector2  > : fastgltf::ElementTraitsBase< Engine::Vector2,  AccessorType::Vec2, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3  > : fastgltf::ElementTraitsBase< Engine::Vector3,  AccessorType::Vec3, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4  > : fastgltf::ElementTraitsBase< Engine::Vector4,  AccessorType::Vec4, float> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2D > : fastgltf::ElementTraitsBase< Engine::Vector2D, AccessorType::Vec2, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3D > : fastgltf::ElementTraitsBase< Engine::Vector3D, AccessorType::Vec3, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4D > : fastgltf::ElementTraitsBase< Engine::Vector4D, AccessorType::Vec4, double> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2U > : fastgltf::ElementTraitsBase< Engine::Vector2U, AccessorType::Vec2, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3U > : fastgltf::ElementTraitsBase< Engine::Vector3U, AccessorType::Vec3, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4U > : fastgltf::ElementTraitsBase< Engine::Vector4U, AccessorType::Vec4, unsigned int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector2I > : fastgltf::ElementTraitsBase< Engine::Vector2I, AccessorType::Vec2, int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector3I > : fastgltf::ElementTraitsBase< Engine::Vector3I, AccessorType::Vec3, int> {};
template <>
struct fastgltf::ElementTraits< Engine::Vector4I > : fastgltf::ElementTraitsBase< Engine::Vector4I, AccessorType::Vec4, int> {};

namespace Engine
{
	bool LoadMesh( const fastgltf::Asset& gltf_asset, const fastgltf::Mesh& gltf_mesh,
                   Model::MeshGroup& mesh_group_to_load, std::vector< Mesh >& meshes, const std::vector< Texture* >& textures )
    {
		/* Naming variables sub-mesh instead of gltf's "primitive" for better readibility. */

        mesh_group_to_load.sub_meshes.reserve( gltf_mesh.primitives.size() );
        mesh_group_to_load.name = gltf_mesh.name;

		for( auto submesh_iterator = gltf_mesh.primitives.begin(); submesh_iterator != gltf_mesh.primitives.end(); ++submesh_iterator )
        {
            std::size_t base_color_uv_index = 0;
            std::size_t material_index      = -1;

            Texture* sub_mesh_albedo_texture    = nullptr;
            Texture* sub_mesh_normal_texture    = nullptr;
            Texture* metallic_roughness_texture = nullptr;
            std::optional< Color3 > sub_mesh_albedo_color;

			auto* position_iterator = submesh_iterator->findAttribute( "POSITION" );
			ASSERT_DEBUG_ONLY( position_iterator != submesh_iterator->attributes.end() ); // A gltf mesh primitive is required to hold the POSITION attribute.

            if( submesh_iterator->materialIndex.has_value() )
            {
                material_index       = submesh_iterator->materialIndex.value();
                const auto& material = gltf_asset.materials[ material_index ];
                
                if( const auto& base_color_texture_info = material.pbrData.baseColorTexture; 
                    base_color_texture_info.has_value() )
                {
                    const auto& fastgltf_texture = gltf_asset.textures[ base_color_texture_info->textureIndex ];
                    if( !fastgltf_texture.imageIndex.has_value() )
                        return false;

                    sub_mesh_albedo_texture = textures[ fastgltf_texture.imageIndex.value() ];
                    if( sub_mesh_albedo_texture->Name().front() == '<' ) // <unnamed>.
                    {
                        std::string old_name( sub_mesh_albedo_texture->Name() );
                        sub_mesh_albedo_texture->SetName( gltf_mesh.name.empty()
														  ? ( "Unnamed Model " + sub_mesh_albedo_texture->Name().substr( 10 ) + " (Albedo)" ).c_str()
                                                          : ( gltf_mesh.name + " (Albedo)" ).c_str() );
                        AssetDatabase< Texture >::RenameAsset( std::move( old_name ), sub_mesh_albedo_texture->Name() );
                    }

                    if( base_color_texture_info->transform && base_color_texture_info->transform->texCoordIndex.has_value() )
                        base_color_uv_index = base_color_texture_info->transform->texCoordIndex.value();
                    else
                        base_color_uv_index = material.pbrData.baseColorTexture->texCoordIndex;
                }
                else
                {
                    // Use Albedo color.
                    sub_mesh_albedo_color = reinterpret_cast< const Color3& /* Omit alpha */ >( material.pbrData.baseColorFactor );
                }

                if( const auto& normal_texture_info = material.normalTexture;
                    normal_texture_info.has_value() )
                {
                    const auto& fastgltf_texture = gltf_asset.textures[ normal_texture_info->textureIndex ];
                    if( !fastgltf_texture.imageIndex.has_value() )
                        return false;

                    sub_mesh_normal_texture = textures[ fastgltf_texture.imageIndex.value() ];
                    if( sub_mesh_normal_texture->Name().front() == '<' ) // <unnamed>.
                    {
                        std::string old_name( sub_mesh_normal_texture->Name() );
                        sub_mesh_normal_texture->SetName( gltf_mesh.name.empty()
                                                          ? ( "Unnamed Model " + sub_mesh_normal_texture->Name().substr( 10 ) + " (Normal)" ).c_str()
                                                          : ( gltf_mesh.name + " (Normal)" ).c_str() );
                        AssetDatabase< Texture >::RenameAsset( std::move( old_name ), sub_mesh_normal_texture->Name() );
                    }
                }

                if( const auto& metallic_rougness_texture_info = material.pbrData.metallicRoughnessTexture;
                    metallic_rougness_texture_info.has_value() )
                {
                    const auto& fastgltf_texture = gltf_asset.textures[ metallic_rougness_texture_info->textureIndex ];
                    if( !fastgltf_texture.imageIndex.has_value() )
                        return false;

                    metallic_roughness_texture = textures[ fastgltf_texture.imageIndex.value() ];
                    if( metallic_roughness_texture->Name().front() == '<' ) // <unnamed>.
                    {
                        std::string old_name( metallic_roughness_texture->Name() );
                        metallic_roughness_texture->SetName( gltf_mesh.name.empty()
                                                             ? ( "Unnamed Model " + metallic_roughness_texture->Name().substr( 10 ) + " (Metallic-Roughness)" ).c_str()
                                                             : ( gltf_mesh.name + " (Metallic-Roughness)" ).c_str() );
                        AssetDatabase< Texture >::RenameAsset( std::move( old_name ), metallic_roughness_texture->Name() );
                    }
                }
            }

            /*
             * Positions:
             */

            /* glTF uses a right-handed coordinate system where x points to right, y points to up & z points from the screen to the user.
	         * Compared to the coordinate system used in this engine, only the Z component is the inverse, x & y are the same. */
	        static Matrix3x3 coordinate_system_transform( Vector3::Right(),
														  Vector3::Up(),
														  Vector3::Backward() );

			const auto& position_accessor = gltf_asset.accessors[ position_iterator->accessorIndex ];
			if( !position_accessor.bufferViewIndex.has_value() )
				continue;

            std::vector< Vector3 > positions( position_accessor.count, ZERO_INITIALIZATION );

			fastgltf::iterateAccessorWithIndex< Vector3 >( gltf_asset, position_accessor,
														   [ & ]( Vector3 position, std::size_t index )
														   {
                                                    	   	   positions[ index ] = position * coordinate_system_transform;
														   } );

            /*
             * Normals:
             */

            std::vector< Vector3 > normals;

			if( auto* normal_iterator = submesh_iterator->findAttribute( "NORMAL" );
                normal_iterator != submesh_iterator->attributes.end() )
            {
                const auto& normal_accessor = gltf_asset.accessors[ normal_iterator->accessorIndex ];
                if( !normal_accessor.bufferViewIndex.has_value() )
                    continue;

                normals.resize( normal_accessor.count, ZERO_INITIALIZATION );

                fastgltf::iterateAccessorWithIndex< Vector3 >( gltf_asset, normal_accessor,
														   [ & ]( Vector3 normal, std::size_t index )
														   {
                                                    	   	   normals[ index ] = normal * coordinate_system_transform;
														   } );
            }

            /*
             * UVs (0):
             */

            std::vector< Vector2 > uvs_0;

            if( auto* uvs_0_iterator = submesh_iterator->findAttribute( "TEXCOORD_0" );
                uvs_0_iterator != submesh_iterator->attributes.end() )
            {
                const auto& uvs_0_accessor = gltf_asset.accessors[ uvs_0_iterator->accessorIndex ];
                if( !uvs_0_accessor.bufferViewIndex.has_value() )
                    continue;

                uvs_0.resize( uvs_0_accessor.count, ZERO_INITIALIZATION );

                fastgltf::copyFromAccessor< Vector2 >( gltf_asset, uvs_0_accessor, uvs_0.data() );
            }

            /*
             * Tangents:
             */

            std::vector< Vector3 > tangents;

			if( auto* tangent_iterator = submesh_iterator->findAttribute( "TANGENT" );
                tangent_iterator != submesh_iterator->attributes.cend() )
            {
                const auto& tangent_accessor = gltf_asset.accessors[ tangent_iterator->accessorIndex ];
                if( !tangent_accessor.bufferViewIndex.has_value() )
                    continue;

                tangents.resize( tangent_accessor.count, ZERO_INITIALIZATION );

                fastgltf::iterateAccessorWithIndex< Vector4 >( gltf_asset, tangent_accessor,
														   [ & ]( Vector4 tangent, std::size_t index )
														   {
                                                    	   	   tangents[ index ] = tangent.XYZ() * coordinate_system_transform;
														   } );
            }

            /*
             * Indices:
             */

			ASSERT_DEBUG_ONLY( submesh_iterator->indicesAccessor.has_value() ); // We specify GenerateMeshIndices, so we should always have indices.

            const auto& index_accessor = gltf_asset.accessors[ submesh_iterator->indicesAccessor.value() ];
            if( !index_accessor.bufferViewIndex.has_value() )
                return false;
            const std::uint32_t index_count = static_cast< std::uint32_t >( index_accessor.count );

            std::vector< std::uint32_t > indices_u32;

            // Ignore 16 bit indices (or any other format other than 32 bit for that matter).
            indices_u32.resize( index_count );

            auto EffectiveIndex = []( const std::size_t index )
            {
                /* To swap the winding order:
                 * Swap the 2nd & 3rd vertex of every triangle to convert from ccw front-faces to cw front-faces. */
                const std::size_t index_mod_3 = index % 3;
                const std::size_t needs_swap( ( bool )index_mod_3 ); // true/1 for 1 & 2, false/0 for 0.
                const std::size_t swapped_index = ( index - index_mod_3 ) + 3 - index_mod_3;
                return needs_swap * swapped_index + ( 1 - needs_swap ) * index;
            };

            fastgltf::iterateAccessorWithIndex< std::uint32_t >( gltf_asset, index_accessor,
                                                                 [ & ]( std::uint32_t actual_index, std::size_t array_index )
                                                                 {
                                                                     indices_u32[ EffectiveIndex( array_index ) ] = actual_index;
                                                                 } );
            
            /* Calculate tangents if the model did not have them. */
            if( tangents.empty() )
            {
                const auto index_count = indices_u32.size();
                const auto size = uvs_0.size();
                tangents.reserve( size );
                for( auto base_index = 0; base_index < index_count; base_index += 3 )
                {
                    const auto index_0 = indices_u32[ base_index     ];
                    const auto index_1 = indices_u32[ base_index + 1 ];
                    const auto index_2 = indices_u32[ base_index + 2 ];

                    const auto position_0 = positions[ index_0 ];
                    const auto position_1 = positions[ index_1 ];
                    const auto position_2 = positions[ index_2 ];

                    const auto uv_0 = uvs_0[ index_0 ];
                    const auto uv_1 = uvs_0[ index_1 ];
                    const auto uv_2 = uvs_0[ index_2 ];

                    const auto edge_1 = position_1 - position_0;
                    const auto edge_2 = position_2 - position_0;

                    const auto delta_uv_1 = uv_1 - uv_0;
                    const auto delta_uv_2 = uv_2 - uv_0;

                    const auto delta_v_1 = delta_uv_1.Y();
                    const auto delta_v_2 = delta_uv_2.Y();

                    const auto f = 1.0f / ( delta_uv_1.X() * delta_v_2 - delta_uv_2.X() * delta_v_1 );

                    tangents.emplace_back( f * ( delta_v_2 * edge_1.X() - delta_v_1 * edge_2.X() ),
                                           f * ( delta_v_2 * edge_1.Y() - delta_v_1 * edge_2.Y() ),
                                           f * ( delta_v_2 * edge_1.Z() - delta_v_1 * edge_2.Z() ) );
                }
            }

            std::string sub_mesh_name( mesh_group_to_load.name + "_" + std::to_string( std::distance( gltf_mesh.primitives.begin(), submesh_iterator ) ) );

			mesh_group_to_load.sub_meshes.emplace_back( sub_mesh_name,
												        /* Actual Mesh will be stored inside the meshes vector. SubMesh will have a reference to this Mesh. */
														meshes.emplace_back( Mesh( std::move( positions ),
																				   sub_mesh_name,
																				   std::move( normals ),
                                                                                   std::move( uvs_0 ),
                                                                                   std::move( indices_u32 ),
                                                                                   std::move( tangents ) ) ),
														sub_mesh_albedo_texture,
                                                        sub_mesh_normal_texture,
                                                        std::move( sub_mesh_albedo_color ) );
		}

        return true;
    }

    bool LoadTexture( const fastgltf::Asset& gltf_asset, const fastgltf::Image& gltf_image,
                      Texture*& texture_to_load, const Engine::Texture::ImportSettings& import_settings )
    {
        std::visit( fastgltf::visitor
                    {
                        []( const auto& arg ) {},
                        [ & ]( const fastgltf::sources::URI& file_path )
                        {
                            ASSERT_DEBUG_ONLY( file_path.fileByteOffset == 0 ); // Offsets with stbi are not supported.
                            ASSERT_DEBUG_ONLY( file_path.uri.isLocalPath() );   // Only capable of loading local files.

                            const std::string path( file_path.uri.path().begin(), file_path.uri.path().end() );

                            texture_to_load = AssetDatabase< Texture >::CreateAssetFromFile( std::string( gltf_image.name ), path, import_settings );
                        },
                        [ & ]( const fastgltf::sources::Array& vector )
                        {
                            texture_to_load = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( gltf_image.name ), vector.bytes.data(), static_cast< int >( vector.bytes.size() ),
																					           false, import_settings );
                        },
                        [ & ]( const fastgltf::sources::BufferView& view )
                        {
                            auto& buffer_view = gltf_asset.bufferViews[ view.bufferViewIndex ];
                            auto& buffer      = gltf_asset.buffers[ buffer_view.bufferIndex ];

                            /* Yes, we've already loaded every buffer into some GL buffer. 
                             * However, with GL it's simpler to just copy the buffer data again for the texture.
                             * Besides, this is just an example. */
                            std::visit( fastgltf::visitor
                                        {
                                            // We only care about VectorWithMime here, because we specify LoadExternalBuffers, meaning all buffers are already loaded into a vector.
                                            []( const auto& arg ) {},
                                            [ & ]( const fastgltf::sources::Array& vector )
                                            {
												texture_to_load = AssetDatabase< Texture >::CreateAssetFromMemory( std::string( gltf_image.name ),
																										           vector.bytes.data() + buffer_view.byteOffset,
																										           static_cast< int >( buffer_view.byteLength ),
																										           false, import_settings );
                                            }
                                        }, buffer.data );
                        }
                    }, gltf_image.data );

        return texture_to_load;
    }

    bool LoadNode( const fastgltf::Node& gltf_node,
                   std::vector< Model::MeshGroup >& mesh_groups, Model::Node& node_to_load )
    {
        /* glTF uses a right-handed coordinate system where x points to right, y points to up & z points from the screen to the user.
         * Compared to the coordinate system used in this engine, only the Z component is the inverse, x & y are the same. 
         
         * To transform matrices, not vectors, we need the inverse of this transform, which happens to be the same exact matrix.
         * So, to transform transformation matrices to our coordinate system, we need to "sandwich" the transformation matrix between 2 of this same matrix. */
		static Matrix4x4 coordinate_system_transform( Vector4::Right(),
													  Vector4::Up(),
													  Vector4::Backward(),
													  Vector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

        const Matrix4x4 node_transform = std::visit( fastgltf::visitor
                                                     {
                                                         []( const fastgltf::TRS& trs ) -> Matrix4x4
                                                         {
                                                             return
                                                                 coordinate_system_transform *
                                                                          Matrix::Scaling( reinterpret_cast< const Vector3&     >( trs.scale ) ) *
                                                                 Math::QuaternionToMatrix( reinterpret_cast< const Quaternion&  >( trs.rotation ) ) *
                                                                      Matrix::Translation( reinterpret_cast< const Vector3&     >( trs.translation ) ) *
                                                                 coordinate_system_transform;
                                                         },
                                                         []( const fastgltf::math::fmat4x4& matrix ) -> Matrix4x4
                                                         {
                                                             // Need to transpose because fastgltf has column-major matrices.
                                                             return coordinate_system_transform * reinterpret_cast< const Matrix4x4& >( matrix ).Transposed() * coordinate_system_transform;
                                                         }
                                                     }, gltf_node.transform );
        
        node_to_load = Model::Node( std::string( gltf_node.name ), node_transform, gltf_node.meshIndex ? &mesh_groups[ *gltf_node.meshIndex ] : nullptr );

        return true;
    }

    std::optional< Model > Model::Loader::FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings )
	{
        fastgltf::Asset gltf_asset;

        // Parse the glTF file and get the constructed asset:
        {
            static constexpr auto supported_extensions =
                fastgltf::Extensions::KHR_mesh_quantization |
                fastgltf::Extensions::KHR_texture_transform |
                fastgltf::Extensions::KHR_materials_variants;

            fastgltf::Parser parser( supported_extensions );

            constexpr auto gltf_options =
                fastgltf::Options::DontRequireValidAssetMember |
                fastgltf::Options::LoadExternalBuffers |
                fastgltf::Options::LoadExternalImages |
                fastgltf::Options::GenerateMeshIndices;

			auto gltfFile = fastgltf::GltfDataBuffer::FromPath( file_path );
			if( !bool( gltfFile ) )
            {
				std::cerr << "ERROR::MODELLOADER::FASTGLTF::Failed to open glTF file: " << fastgltf::getErrorMessage( gltfFile.error() ) << '\n';
				return std::nullopt;
			}

            std::filesystem::path path( file_path );

            fastgltf::Expected< fastgltf::Asset > maybe_gltf_asset( fastgltf::Error::None );
             
            if( const auto gltf_type = fastgltf::determineGltfFileType( gltfFile.get() );
                gltf_type == fastgltf::GltfType::glTF )
                maybe_gltf_asset = parser.loadGltf( gltfFile.get(), path.parent_path(), gltf_options );
            else
                maybe_gltf_asset = parser.loadGltfBinary( gltfFile.get(), path.parent_path(), gltf_options );

            if( maybe_gltf_asset.error() != fastgltf::Error::None )
            {
                std::cerr << "ERROR::MODELLOADER::FASTGLTF::Failed to load glTF: " << fastgltf::getErrorMessage( maybe_gltf_asset.error() ) << '\n';
				return std::nullopt;
			}

            gltf_asset = std::move( maybe_gltf_asset.get() );
        }

        Model model( std::string{ name } );
        
        model.textures.reserve( gltf_asset.images.size() );

        /* For the LoadMesh() to work properly, textures in Model need to be ordered based on gltf_asset.images, NOT gltf_asset.textures (they can have different orderings). 
         * While it would be more convenient to loop of gltf_asset.textures here since it has an imageIndex into the gltf_asset.images array, we can't,
         * as we need to create textures in the order of gltf_asset.images.
         *
         * Therefore, we first cache the mapping between the two arrays: */
        std::unordered_map< std::size_t, std::size_t > gltf_texture_index_from_image_index;
        for( std::size_t texture_index = 0; texture_index < gltf_asset.textures.size(); texture_index++ )
            gltf_texture_index_from_image_index[ *gltf_asset.textures[ texture_index ].imageIndex ] = texture_index;

        /* Now we can loop over the images safely, referring to the texture index from the map as needed. */
        for( std::size_t image_index = 0; image_index < gltf_asset.images.size(); image_index++ )
        {
            const auto& gltf_image = gltf_asset.images[ image_index ];

            const std::size_t texture_index = gltf_texture_index_from_image_index[ image_index ];
            const auto& gltf_texture = gltf_asset.textures[ texture_index ];

            Texture::ImportSettings import_settings
            {
                .wrap_u          = Texture::Wrapping::Repeat,
                .wrap_v          = Texture::Wrapping::Repeat,
                /* gltf spec dictates that V coordinates increase in downward direction (i.e., UV origin is top-left).
                 * This is compatible with how stb interprets/treats uvs, so no need to flip uvs coming from gltf. */
                .flip_vertically = false
            };

            if( gltf_texture.samplerIndex.has_value() )
            {
                const auto& sampler = gltf_asset.samplers[ *gltf_texture.samplerIndex ];
                import_settings.wrap_u = Texture::Wrapping( sampler.wrapS );
                import_settings.wrap_v = Texture::Wrapping( sampler.wrapT );
                if( sampler.minFilter.has_value() )
                    import_settings.min_filter = Texture::Filtering( *sampler.minFilter );
                if( sampler.magFilter.has_value() )
                    import_settings.mag_filter = Texture::Filtering( *sampler.magFilter );

            }

            /* Turn off sRGB status for linear textures: */
            for( auto& material : gltf_asset.materials )
            {
                if( ( material.normalTexture                            && material.normalTexture->textureIndex                        == texture_index ) ||
                    ( material.packedNormalMetallicRoughnessTexture     && material.packedNormalMetallicRoughnessTexture->textureIndex == texture_index ) ||
                    ( material.occlusionTexture                         && material.occlusionTexture->textureIndex                     == texture_index ) ||
                    ( material.packedOcclusionRoughnessMetallicTextures &&
					  ( ( material.packedOcclusionRoughnessMetallicTextures->normalTexture &&
						  material.packedOcclusionRoughnessMetallicTextures->normalTexture->textureIndex == texture_index ) ||
						( material.packedOcclusionRoughnessMetallicTextures->occlusionRoughnessMetallicTexture &&
						  material.packedOcclusionRoughnessMetallicTextures->occlusionRoughnessMetallicTexture->textureIndex == texture_index ) ||
						( material.packedOcclusionRoughnessMetallicTextures->roughnessMetallicOcclusionTexture &&
						  material.packedOcclusionRoughnessMetallicTextures->roughnessMetallicOcclusionTexture->textureIndex == texture_index ) ) ) )
                {
                    import_settings.format = Texture::Format::RGBA;
                }
            }

            if( not LoadTexture( gltf_asset, gltf_image,
                                 model.textures.emplace_back(), import_settings ) )
                return std::nullopt;
        }

        /*
         * Mapping between the glTF & this engine: 
         * scene        -> ~Model (Default scene is assumed, multiple scenes are not supported).
         * node         -> Node
         * mesh         -> MeshGroup (Node has a MeshGroup pointer)
         * primitive    -> SubMesh (MeshGroup has a vector of SubMeshes).
         */

        model.mesh_groups.resize( gltf_asset.meshes.size() ); // Node points to an optional MeshGroup, MeshGroup vector has to be provided at this time.

        model.nodes.reserve( gltf_asset.nodes.size() );

        /* Weird gltf terminology; A "mesh" in glTF is actually just a *group*, whereas the "primitive" is the actual mesh, nested inside a "mesh".
         * A better name for a primitive would be sub-mesh.
         */

        const auto sub_mesh_count = std::accumulate( gltf_asset.meshes.cbegin(), gltf_asset.meshes.cend(), 0,
													 []( const int sum_so_far, const auto& gltf_mesh )
		                                             {
			                                             return sum_so_far + ( int )gltf_mesh.primitives.size();
		                                             } );

        model.meshes.reserve( sub_mesh_count );

        std::copy( gltf_asset.scenes.front().nodeIndices.cbegin(), gltf_asset.scenes.front().nodeIndices.cend(), std::back_inserter( model.node_indices_top_level ) );

        for( auto index = 0; index < gltf_asset.nodes.size(); index++ )
        {
            const auto& gltf_node = gltf_asset.nodes[ index ];
            auto& node            = model.nodes.emplace_back();

            if( not LoadNode( gltf_node,
							  model.mesh_groups, node ) )
                return std::nullopt;

            node.children.reserve( gltf_node.children.size() );
            for( auto& child_index : gltf_node.children )
                node.children.push_back( ( int )child_index );

            if( node.mesh_group )
            {
                if( not LoadMesh( gltf_asset, gltf_asset.meshes[ *gltf_node.meshIndex ],
								  *node.mesh_group, model.meshes, model.textures ) )
                    return std::nullopt;

                model.mesh_instance_count += ( int )node.mesh_group->sub_meshes.size();
            }
        }

        return model;
	}
}