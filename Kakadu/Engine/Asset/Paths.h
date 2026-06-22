#pragma once

#if __has_include( "../../Generated/EngineAssetAbsolutePath.h" )
#  include "../../Generated/EngineAssetAbsolutePath.h"
#else
#define ENGINE_ASSET_ROOT_ABSOLUTE R"(to-be-generated)"
#endif

#define ENGINE_SHADER_ROOT_ABSOLUTE		ENGINE_ASSET_ROOT_ABSOLUTE	"/Shader"
#define ENGINE_TEXTURE_ROOT_ABSOLUTE	ENGINE_ASSET_ROOT_ABSOLUTE	"/Texture"
#define ENGINE_FONT_ROOT_ABSOLUTE		ENGINE_ASSET_ROOT_ABSOLUTE	"/Font"

// With slashes:
#define ENGINE_SHADER_PATH_ABSOLUTE( filename_with_extension )	ENGINE_SHADER_ROOT_ABSOLUTE		"/" filename_with_extension
#define ENGINE_TEXTURE_PATH_ABSOLUTE( filename_with_extension ) ENGINE_TEXTURE_ROOT_ABSOLUTE	"/" filename_with_extension
#define ENGINE_FONT_PATH_ABSOLUTE( filename_with_extension )	ENGINE_FONT_ROOT_ABSOLUTE		"/" filename_with_extension
