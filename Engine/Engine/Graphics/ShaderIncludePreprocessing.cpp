// Engine Includes.
#include "ShaderIncludePreprocessing.h"
#include "Core/Utility.hpp"

namespace Engine::ShaderIncludePreprocessing
{
	std::string Resolve( const std::filesystem::path& source_path, std::initializer_list< std::string > include_directories_with_trailing_slashes )
	{
		std::string source;

		if( auto maybe_file = Utility::ReadFileIntoString( source_path.string().c_str() );
			not maybe_file )
			return ""; // File could not be opened.
		else
			source = std::move( *maybe_file );

		std::string processed;

		std::size_t prev_pos = 0;
		std::size_t curr_pos = source.find( "#include" );

		if( curr_pos == std::string_view::npos )
			return source; // No #includes in file.

		/* Check for the #extension line and skip it: */
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
					break;
				}

				extension_pos = source_view.find( "#extension" );
			}
		}

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

			/* Process the current #include. */
			{
				/* Find the file: */
				std::filesystem::path file_path;
				bool file_found = false;

				/* Always check the source directory first: */
				if( not ( file_found = std::filesystem::exists( file_path = source_path.parent_path() / file_name_view ) ) )
				{
					for( const auto& include_directory : include_directories_with_trailing_slashes )
						if( file_found = std::filesystem::exists( file_path = include_directory + std::string( file_name_view ) ) )
							break;
				}

				if( not file_found )
					return ""; // File not found.

				/* The #include may have other #includes: */
				const auto resolved = Resolve( file_path, include_directories_with_trailing_slashes );
				if( resolved.empty() )
					return ""; // #include inside the #include could not be resolved.

				processed += resolved + "\n";
			}

			// Look for the next #include.
			prev_pos = end_of_line_pos + 1;
			curr_pos = source.find( "#include", prev_pos );
		}

		processed += std::string_view( source.data() + prev_pos );

		return processed;
	}
}
