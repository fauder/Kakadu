// Engine Includes.
#include "ShaderIncludePreprocessing.h"
#include "Core/Utility.hpp"

// TODO: Pre-processing error logging via a char* error_log parameter.

namespace Engine::ShaderIncludePreprocessing
{
	/* Implementation Function prototype: */
	std::string Resolve( const std::filesystem::path& source_path,
						 std::initializer_list< std::string > include_directories_with_trailing_slashes,
						 std::unordered_map< std::filesystem::path, std::int16_t >& map_of_source_file_per_ID );

	/* Public API: */
	std::string Resolve( const std::filesystem::path& source_path,
						 std::initializer_list< std::string > include_directories_with_trailing_slashes,
						 std::unordered_map< std::int16_t, std::filesystem::path >& map_of_IDs_per_source_file )
	{
		/* Map provided by the caller is for caller's use (i.e., they need to refer to the source file corresponding to the key in shader compilation error logs.). 
		 * The implementation on the other hand needs the inverse map: To assign and look-up indices based on shader source file path. */
		std::unordered_map< std::filesystem::path, std::int16_t > map_of_source_file_per_ID;
		map_of_source_file_per_ID[ source_path ] = 0; // The main source file is always the ID 0.

		const std::string resolved_source( Resolve( source_path, include_directories_with_trailing_slashes, map_of_source_file_per_ID ) );

		/* Remap: */
		for( auto& [ source_file_path, file_ID ] : map_of_source_file_per_ID )
			map_of_IDs_per_source_file[ file_ID ] = source_file_path;

		return resolved_source;
	}

	/* Implementation Function definition: */
	std::string Resolve( const std::filesystem::path& source_path,
						 std::initializer_list< std::string > include_directories_with_trailing_slashes,
						 std::unordered_map< std::filesystem::path, std::int16_t >& map_of_source_file_per_ID )
	{
		std::string source;

		if( auto maybe_file = Utility::ReadFileIntoString( source_path.string().c_str() );
			not maybe_file )
			return ""; // File could not be opened.
		else
			source = std::move( *maybe_file );

		std::string processed;

		std::size_t curr_pos                   = source.find( "#include" );
		std::size_t prev_pos                   = 0;
		std::size_t last_line_number_cache_pos = 0;

		if( curr_pos == std::string_view::npos )
			return source; // No #includes in file.

		std::uint16_t offset_for_extension_line = 0;
		std::uint16_t line_count_cached = 0;

		/* Check for the #extension line and skip it, but only for actual sources, not headers: */
		constexpr std::array< std::string_view, 7 > valid_source_extensions{ ".glsl", ".frag", ".vert", ".geom", ".comp", ".tese", ".tesc" };
		if( std::find( valid_source_extensions.begin(), valid_source_extensions.end(), source_path.extension() ) != valid_source_extensions.end() )
		{
			std::string_view source_view( source );
			auto extension_pos = source_view.find( "#extension" );
			while( extension_pos != std::string_view::npos )
			{
				const auto end_of_line_pos = source_view.find( "\n", extension_pos );
				if( auto pos = source_view.find( "GL_ARB_shading_language_include" ); // Ignoring the edge cases where the extension is disabled etc.
					pos < end_of_line_pos )
				{
					/* Append the lines up until the #extension line. */
					processed += std::string_view( source.data(), extension_pos );

					/* Skip this line. */
					prev_pos = curr_pos = source.find( "#include", end_of_line_pos + 1 );

					offset_for_extension_line = 1; // This way, we can account for the #extension line when using line numbers.

					break;
				}

				extension_pos = source_view.find( "#extension" );
			}
		}

		/* Main #include processing loop. */
		while( curr_pos != std::string_view::npos )
		{
			const std::size_t after_include_token_pos = curr_pos + ( sizeof( "#include " ) - 1 );

			const std::size_t end_of_line_pos = source.find( "\n", after_include_token_pos );
			//const std::string_view line_view( &source[ curr_pos ], &source[ end_of_line_pos ] );

			const std::size_t end_of_file_name_pos = source.find( ".glsl", after_include_token_pos ) + ( sizeof( ".glsl" ) - 1 );
			std::string_view file_name_view( &source[ after_include_token_pos ], &source[ end_of_file_name_pos ] );

			file_name_view = Utility::String::RemoveLeadingWhitespace( file_name_view );
			if( file_name_view.starts_with( '"' ) )
				file_name_view.remove_prefix( 1 );

			/* Append the lines up until the #include line. */
			processed += source.substr( prev_pos, curr_pos - prev_pos );

			/* Process the current #include ONLY IF it has not been processed before. */
			/* Find the file: */
			std::filesystem::path include_file_path;
			bool include_file_found = false;

			/* Always check the source directory first: */
			if( not ( include_file_found = std::filesystem::exists( include_file_path = source_path.parent_path() / file_name_view ) ) )
			{
				for( const auto& include_directory : include_directories_with_trailing_slashes )
					if( include_file_found = std::filesystem::exists( include_file_path = include_directory + std::string( file_name_view ) ) )
						break;
			}

			if( not include_file_found )
				return ""; // File not found.

			if( not map_of_source_file_per_ID.contains( include_file_path ) )
			{
				const  std::int16_t  current_include_file_path_ID = ( std::int16_t )map_of_source_file_per_ID.size();
				map_of_source_file_per_ID.insert( { include_file_path, current_include_file_path_ID } );

				/* The #include may have other #includes: */
				const auto resolved = Resolve( include_file_path, include_directories_with_trailing_slashes, map_of_source_file_per_ID );
				if( resolved.empty() )
					return ""; // #include inside the #include could not be resolved.

				// Insert #line for correct error reporting during validation & compilation.
				line_count_cached += 1 + std::count( source.begin() + last_line_number_cache_pos, source.begin() + end_of_line_pos, '\n' ); // +1 for the current #include line.
				last_line_number_cache_pos = end_of_line_pos + 1;

				processed += "#line 0 " + std::to_string( current_include_file_path_ID ) + "\n";

				processed += resolved;

				if( resolved.back() != '\n' )
					processed += '\n';

				/* Do not restore the #line for the parent when there are consecutive #include lines.
				 * Therefore, look for the next #include first. */
				prev_pos = end_of_line_pos + 1;
				curr_pos = source.find( "#include", prev_pos );

				if( curr_pos != prev_pos )
				{
					// Restore #line back for the parent.
					processed += "#line " + std::to_string( line_count_cached + 1 ) + " " + std::to_string( map_of_source_file_per_ID[ source_path ] ) + "\n";
				}
			}
			else // Skip the already accounted-for #include:
			{
				// Look for the next #include.
				prev_pos = end_of_line_pos + 1;
				curr_pos = source.find( "#include", prev_pos );
			}
		}

		processed += std::string_view( source.data() + prev_pos );

		return processed;
	}
}
