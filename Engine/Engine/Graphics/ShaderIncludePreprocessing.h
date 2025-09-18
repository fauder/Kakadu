#pragma once

// std Includes.
#include <filesystem>

namespace Engine::ShaderIncludePreprocessing
{
	std::string Resolve( const std::filesystem::path& source_path, std::initializer_list< std::string > include_directories_with_trailing_slashes );
}
