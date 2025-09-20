#pragma once

// std Includes.
#include <filesystem>
#include <span>
#include <unordered_map>

namespace Engine::ShaderIncludePreprocessing
{
	std::string Resolve( const std::filesystem::path& source_path, 
						 std::initializer_list< std::string > include_directories_with_trailing_slashes,
						 std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file,
						 std::span< char > error_log );
}
