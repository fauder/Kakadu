#pragma once

// Engine Includes.
#include "RHI/Buffer.h"
#include "RHI/Uniform.h"

// std Includes.
#include <unordered_map>

namespace Kakadu
{
	// Singleton.
	class UniformBufferManager
	{
	public:
		DELETE_COPY_AND_MOVE_CONSTRUCTORS( UniformBufferManager );

		static RHI::Buffer* CreateOrRequest( const std::string& buffer_name, const RHI::Uniform::BufferInformation& buffer_info );

	private:
		UniformBufferManager() {}

		/* Singleton related: */
		static UniformBufferManager& Instance()
		{
			local_persist UniformBufferManager instance;
			return instance;
		}

	private:
		std::unordered_map< std::string, RHI::Buffer > uniform_buffer_map_regular;
		std::unordered_map< std::string, RHI::Buffer > uniform_buffer_map_global;
		std::unordered_map< std::string, RHI::Buffer > uniform_buffer_map_intrinsic;
	};
}
