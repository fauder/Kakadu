#pragma once

// Engine Includes.
#include "Texture.h"
#include "Core/Types.h"

// std Includes.
#include <string>
#include <vector>

namespace Kakadu::RHI::Capabilities
{
	/* Logs a warning if the sample count in question is not available for the given format. */
	bool QueryMSAASupport( const Texture::Format format, const u8 sample_count_to_query );
	void QueryAvailableGLExtensions( std::vector< std::string >& list_of_strings );
	u32 QueryMaximumUniformBufferBindingCount();
}
