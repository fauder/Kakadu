#pragma once

// Engine Includes.
#include "Concepts.h"
#include "Macros.h"

// std Includes.
#include <map>
#include <string>

namespace Engine
{
	template< Concepts::Pointer AssetPointerType >
	class AssetDatabase_Tracked
	{
	public:
		AssetDatabase_Tracked()
		{}

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( AssetDatabase_Tracked );

		AssetPointerType AddAsset( AssetPointerType asset )
		{
			auto [ it, inserted ] = tracked_asset_map.try_emplace( asset->Name(), asset );

			return inserted ? asset : nullptr;
		}

		AssetPointerType AddOrUpdateAsset( AssetPointerType asset )
		{
			return tracked_asset_map[ asset->Name() ] = asset;
		}

		bool RemoveAsset( const std::string& name )
		{
			return tracked_asset_map.erase( name ) > 0;
		}

		bool RenameAsset( const std::string& old_name, const std::string& new_name )
		{
			if( old_name == new_name )
				return true;

			if( tracked_asset_map.contains( new_name ) )
				return false;

			if( auto asset_node = tracked_asset_map.extract( old_name );
				asset_node )
			{
				asset_node.key() = new_name;
				tracked_asset_map.insert( std::move( asset_node ) );

				return true;
			}

			return false;
		}

		const std::map< std::string, AssetPointerType >& Assets() const
		{
			return tracked_asset_map;
		}

	private:
		// TODO: Implement & integrate GUID generation.

		std::map< std::string /* Key is name (for now) */, AssetPointerType > tracked_asset_map;
	};
}
