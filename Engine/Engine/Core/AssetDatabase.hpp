#pragma once

// Engine Includes.
#include "Macros.h"

// std Includes.
#include <map>
#include <string>

namespace Engine
{
	// Singleton.
	template< typename AssetType >
	class AssetDatabase
	{
	public:
		DELETE_COPY_AND_MOVE_CONSTRUCTORS( AssetDatabase );

		static AssetType* CreateAssetFromFile( const std::string& name,
											   const std::string& file_path,
											   const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto& instance = Instance();

			if( not instance.asset_path_map.contains( file_path ) )
			{
				if( auto maybe_asset = AssetType::Loader::FromFile( name, file_path, import_settings ); 
					maybe_asset )
				{
					instance.asset_map[ name ]      = std::move( *maybe_asset );
					instance.asset_path_map[ name ] = file_path;
				}
				else // Failed to load asset:
					return nullptr;
			}

			/* Asset is already loaded, return the existing one. */
			return &instance.asset_map[ name ];
		}

		/* For assets with mulitple source-assets, such as cubemaps. */
		static AssetType* CreateAssetFromFile( const std::string& name,
											   const std::initializer_list< std::string > file_paths,
											   const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto& instance = Instance();

			if( not instance.asset_path_map.contains( *file_paths.begin() ) ) // TODO: Fix this.
			{
				if( auto maybe_asset = AssetType::Loader::FromFile( name, file_paths, import_settings );
					maybe_asset )
				{
					instance.asset_map[ name ]      = std::move( *maybe_asset );
					instance.asset_path_map[ name ] = *file_paths.begin(); // TODO: Fix this.
				}
				else // Failed to load asset:
					return nullptr;
			}

			/* Asset is already loaded, return the existing one. */
			return &instance.asset_map[ name ];
		}

		static AssetType* CreateAssetFromFileBytes( const std::string& name,
													const std::byte* data,
													const int length,
													const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto& instance = Instance();

			if( name.empty() )
			{
				std::string new_name( "<unnamed>_" + std::to_string( ( int )instance.asset_map.size() ) );

				if( auto maybe_asset = AssetType::Loader::FromFileBytes( new_name, data, length, import_settings );
					maybe_asset )
				{
					instance.asset_map[ new_name ] = std::move( *maybe_asset );
					return &instance.asset_map[ new_name ];
				}

				// Failed to load asset:
				return nullptr;
			}
			else 
			{
				if( not instance.asset_map.contains( name ) ) // Can not compare file_paths as the asset does not & will not have a path.
				{
					if( auto maybe_asset = AssetType::Loader::FromFileBytes( name, data, length, import_settings );
						maybe_asset )
					{
						instance.asset_map[ name ] = std::move( *maybe_asset );
					}
					else // Failed to load asset:
						return nullptr;
				}

				/* Asset is already loaded, return the existing one. */
				return &instance.asset_map[ name ];
			}
		}

		static AssetType* CreateAssetFromRawBytes( const std::string& name,
												   const std::byte* data,
												   const typename AssetType::SizeType size,
												   const typename AssetType::ImportSettings& import_settings = AssetType::DEFAULT_IMPORT_SETTINGS )
		{
			auto& instance = Instance();

			if( name.empty() )
			{
				std::string new_name( "<unnamed>_" + std::to_string( ( int )instance.asset_map.size() ) );

				if( auto maybe_asset = AssetType::Loader::FromRawBytes( new_name, data, size, import_settings );
					maybe_asset )
				{
					instance.asset_map[ new_name ] = std::move( *maybe_asset );
					return &instance.asset_map[ new_name ];
				}

				// Failed to load asset:
				return nullptr;
			}
			else 
			{
				if( not instance.asset_map.contains( name ) ) // Can not compare file_paths as the asset does not & will not have a path.
				{
					if( auto maybe_asset = AssetType::Loader::FromRawBytes( name, data, size, import_settings );
						maybe_asset )
					{
						instance.asset_map[ name ] = std::move( *maybe_asset );
					}
					else // Failed to load asset:
						return nullptr;
				}

				/* Asset is already loaded, return the existing one. */
				return &instance.asset_map[ name ];
			}
		}

		static AssetType* AddAsset( AssetType&& asset,
									const std::string& file_path = "<not-on-disk>" )
		{
			auto& instance = Instance();

			const auto& asset_name = asset.Name();

			/* If the asset is already loaded, return the existing one. */
			if( instance.asset_map.contains( asset_name ) )
				return &instance.asset_map[ asset_name ];

			instance.asset_path_map.emplace( asset_name, file_path );
			return &( instance.asset_map.emplace( asset_name, std::move( asset ) ).first->second );
		}

		static AssetType* AddOrUpdateAsset( AssetType&& asset,
											const std::string& file_path = "<not-on-disk>" )
		{
			auto& instance = Instance();

			const auto& asset_name = asset.Name();

			instance.asset_path_map.try_emplace( asset_name, file_path );
			return &( instance.asset_map.try_emplace( asset_name, std::move( asset ) ).first->second );
		}

		static bool RemoveAsset( const std::string& name )
		{
			auto& instance = Instance();

			bool found_asset = false;

			found_asset |= instance.asset_path_map.erase( name ) > 0;
			found_asset |= instance.asset_map.erase( name ) > 0;

			return found_asset;
		}

		static bool RenameAsset( std::convertible_to< std::string_view > auto&& old_name, std::convertible_to< std::string_view > auto&& new_name )
		{
			auto& instance = Instance();

			if( auto asset_node = instance.asset_map.extract( old_name );
				asset_node )
			{
				asset_node.key() = std::forward< decltype( new_name ) >( new_name );
				instance.asset_map.insert( std::move( asset_node ) );

				if( auto path_node = instance.asset_path_map.extract( old_name );
					path_node )
				{
					path_node.key() = std::forward< decltype( new_name ) >( new_name );
					instance.asset_path_map.insert( std::move( path_node ) );
				}

				return true;
			}

			return false;
		}

		static const std::map< std::string, AssetType >& Assets()
		{
			auto& instance = Instance();
			return instance.asset_map;
		}

	private:
		AssetDatabase()
		{}

		static AssetDatabase& Instance()
		{
			static AssetDatabase instance;
			return instance;
		}

	private:
		// TODO: Implement & integrate GUID generation.

		std::map< std::string /* <-- Name (for now) */, AssetType > asset_map;
		std::map< std::string /* <-- Name (for now) */, std::string > asset_path_map;
	};
}
