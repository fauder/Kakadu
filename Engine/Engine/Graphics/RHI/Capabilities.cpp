// Engine Includes.
#include "Capabilities.h"

// std Includes.
#include <unordered_map>
#include <unordered_set>

namespace Kakadu::RHI::Capabilities
{
	bool QueryMSAASupport( const Texture::Format format, const u8 sample_count_to_query )
	{
		local_persist std::unordered_map< Texture::Format, std::unordered_set< u8 > > SAMPLE_COUNTS_BY_FORMAT_MAP;

		if( const auto iterator = SAMPLE_COUNTS_BY_FORMAT_MAP.find( format ); iterator != SAMPLE_COUNTS_BY_FORMAT_MAP.cend() )
			return iterator->second.contains( sample_count_to_query );

		i32 number_of_sample_counts = 0;
		glGetInternalformativ( GL_RENDERBUFFER, Texture::InternalFormat( format ), GL_NUM_SAMPLE_COUNTS, 1, &number_of_sample_counts );

		std::vector< i32 > sample_counts( number_of_sample_counts );
		glGetInternalformativ( GL_RENDERBUFFER, Texture::InternalFormat( format ), GL_SAMPLES, number_of_sample_counts, sample_counts.data() );
		auto& set_of_sample_counts_queried = SAMPLE_COUNTS_BY_FORMAT_MAP[ format ];
		std::transform( sample_counts.begin(), sample_counts.end(), std::inserter( set_of_sample_counts_queried, set_of_sample_counts_queried.begin() ),
						[]( i32 sample_count ) { return sample_count; } );

		return set_of_sample_counts_queried.contains( sample_count_to_query );
	}

	void QueryAvailableGLExtensions( std::vector< std::string >& list_of_strings )
	{
		GLint num_extensions = 0;
		glGetIntegerv( GL_NUM_EXTENSIONS, &num_extensions );

		list_of_strings.reserve( num_extensions );

		for( GLint i = 0; i < num_extensions; ++i )
		{
			const char* ext = reinterpret_cast< const char* >( glGetStringi( GL_EXTENSIONS, i ) );
			list_of_strings.emplace_back( ext );
		}
	}

	u32 QueryMaximumUniformBufferBindingCount()
	{
		u32 query_result;
		glGetIntegerv( GL_MAX_UNIFORM_BUFFER_BINDINGS, ( i32* )&query_result );
		return query_result;
	}
}
