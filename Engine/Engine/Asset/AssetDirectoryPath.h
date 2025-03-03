#pragma once

#define ENGINE_ASSET_DIRECTORY_MACRO "../../Engine/Engine/Asset"
#define ENGINE_ASSET_DIRECTORY_WITH_SEPARATOR_MACRO "../../Engine/Engine/Asset/"
#define ENGINE_ASSET_PATH( relative_file_path ) ENGINE_ASSET_DIRECTORY_WITH_SEPARATOR_MACRO relative_file_path

namespace Engine
{
    constexpr const char* ENGINE_ASSET_DIRECTORY                = ENGINE_ASSET_DIRECTORY_MACRO;
    constexpr const char* ENGINE_ASSET_DIRECTORY_WITH_SEPARATOR = ENGINE_ASSET_DIRECTORY_WITH_SEPARATOR_MACRO;
}
