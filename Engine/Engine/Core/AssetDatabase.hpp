#pragma once

// Engine Includes.
#include "Concepts.h"
#include "Macros.h"

// std Includes.
#include <cstddef>
#include <map>
#include <string>
#include <unordered_map>

namespace Engine
{
	template< Concepts::NotPointer AssetType >
	class AssetDatabase
	{
	public:
		AssetDatabase()
			:
			unique_name_counter( 0 )
		{}

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( AssetDatabase );

		/* Overwrite/rename is prohibited; returns nullptr on failure. */
		AssetType* CreateAssetFromFile( const std::string& name,
										const std::string& file_path,
										const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			if( asset_map.contains( name ) )
				return nullptr;

			if( reverse_asset_path_map.contains( file_path ) )
				return nullptr;

			if( auto maybe_asset = AssetType::Loader::FromFile( name, file_path, import_settings ); 
				maybe_asset )
			{
				asset_path_map[ name ] = file_path;
				reverse_asset_path_map[ file_path ] = name;
				asset_map[ name ] = std::move( *maybe_asset );
				return &asset_map[ name ];
			}

			// Failed to load asset:
			return nullptr;
		}

		/* For assets with multiple source-assets, such as cubemaps.
		 * Overwrite/rename is prohibited; returns nullptr on failure. */
		AssetType* CreateAssetFromFile( const std::string& name,
										const std::initializer_list< std::string > file_paths,
										const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			if( file_paths.size() == 0 )
				return nullptr;

			const std::string& first_path = *file_paths.begin();

			if( asset_map.contains( name ) )
				return nullptr;

			if( reverse_asset_path_map.contains( first_path ) )
				return nullptr;

			if( auto maybe_asset = AssetType::Loader::FromFile( name, file_paths, import_settings );
				maybe_asset )
			{
				/* If we add all file paths to the reverse path map, we create an asymmetry between the path map and the reverse path map, which can get hairy. */

				asset_path_map[ name ] = first_path;
				reverse_asset_path_map[ first_path ] = name;
				asset_map[ name ] = std::move( *maybe_asset );
				return &asset_map[ name ];
			}

			// Failed to load asset:
			return nullptr;
		}

		/* The 'data' argument here contains the parsed file contents that are still encoded and need to be decoded before actual use.
		 * Overwrite/rename is prohibited; returns nullptr on failure. */
		AssetType* CreateAssetFromFileBytes( const std::string& name,
											 const std::byte* data,
											 const int length,
											 const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto Load = [ & ]( const std::string& asset_name ) -> AssetType*
			{
				if( auto maybe_asset = AssetType::Loader::FromFileBytes( asset_name, data, length, import_settings );
					maybe_asset )
				{
					auto [ it, inserted_for_sure ] = asset_map.emplace( asset_name, std::move( *maybe_asset ) );

					const std::string pseudo_path = "<not-on-disk>:" + asset_name;

					asset_path_map.emplace( asset_name, pseudo_path );
					reverse_asset_path_map.emplace( pseudo_path, asset_name );
					return &( it->second );
				}

				// Failed to load asset:
				return nullptr;
			};

			if( name.empty() )
				return Load( "<unnamed>_" + std::to_string( ( int )unique_name_counter++ ) );
			else
				return Load( name );
		}

		/* The 'data' argument here is already decoded and ready to be consumed directly. */
		AssetType* CreateAssetFromRawBytes( const std::string& name,
											const std::byte* data,
											const typename AssetType::SizeType size,
											const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto Load = [ & ]( const std::string& asset_name ) -> AssetType*
			{
				if( auto maybe_asset = AssetType::Loader::FromRawBytes( asset_name, data, size, import_settings );
					maybe_asset )
				{
					auto [ it, inserted_for_sure ] = asset_map.emplace( asset_name, std::move( *maybe_asset ) );

					const std::string pseudo_path = "<not-on-disk>:" + asset_name;

					asset_path_map.emplace( asset_name, pseudo_path );
					reverse_asset_path_map.emplace( pseudo_path, asset_name );
					return &( it->second );
				}

				// Failed to load asset:
				return nullptr;
			};

			if( name.empty() )
				return Load( "<unnamed>_" + std::to_string( ( int )unique_name_counter++ ) );
			else
				return Load( name );
		}

		bool RemoveAsset( const std::string& name )
		{
			if( auto it = asset_path_map.find( name );
				it != asset_path_map.end() )
			{
				reverse_asset_path_map.erase( it->second );
				asset_path_map.erase( it );
				asset_map.erase( name );

				return true;
			}

			return false;
		}

		bool RenameAsset( const std::string& old_name, const std::string& new_name )
		{
			if( old_name == new_name )
				return true;

			if( asset_map.contains( new_name ) )
				return false;

			if( auto asset_node = asset_map.extract( old_name );
				asset_node )
			{
				asset_node.key() = new_name;
				asset_map.insert( std::move( asset_node ) );

				if( auto path_node = asset_path_map.extract( old_name );
					path_node )
				{
					const std::string& path = path_node.mapped();

					path_node.key() = new_name;
					asset_path_map.insert( std::move( path_node ) );

					ASSERT_DEBUG_ONLY( reverse_asset_path_map.contains( path ) );

					reverse_asset_path_map[ path ] = new_name;
				}

				return true;
			}

			// Asset didn't exist to begin with:
			return false;
		}

		const std::map< std::string, AssetType >& Assets() const
		{
			return asset_map;
		}

	private:
		// TODO: Implement & integrate GUID generation.

		std::map< /* Key is name (for now) */ std::string, AssetType > asset_map;
		std::map< /* Key is name (for now) */ std::string, std::string /* Value is path. */ > asset_path_map;
		std::unordered_map< /* Key is path */ std::string, std::string /* Value is name (for now) */ > reverse_asset_path_map;

		std::size_t unique_name_counter;
	};
}
