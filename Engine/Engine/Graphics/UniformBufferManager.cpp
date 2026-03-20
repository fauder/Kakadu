// Engine Includes.
#include "UniformBufferManager.h"
#include "Core/Optimization.h"
#include "RHI/UniformBlockBindingPointManager.h"

namespace Kakadu
{
	RHI::Buffer* UniformBufferManager::CreateOrRequest( const std::string& buffer_name, const RHI::Uniform::BufferInformation& buffer_info )
	{
		UniformBufferManager& instance = Instance();

		switch( buffer_info.category )
		{
			case RHI::Uniform::BufferCategory::Regular:
			{
				RHI::Buffer& buffer = instance.uniform_buffer_map_regular.try_emplace( /* Key: */ buffer_name,
																					   /* Buffer constructor: */ RHI::BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				RHI::Uniform::BlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, RHI::Uniform::BufferCategory::Regular );

				return &buffer;
			}
			case RHI::Uniform::BufferCategory::Global:
			{
				RHI::Buffer& buffer = instance.uniform_buffer_map_global.try_emplace( /* Key: */ buffer_name,
																					  /* Buffer constructor: */ RHI::BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				RHI::Uniform::BlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, RHI::Uniform::BufferCategory::Global );

				return &buffer;
			}
			case RHI::Uniform::BufferCategory::Intrinsic:
			{
				RHI::Buffer& buffer = instance.uniform_buffer_map_intrinsic.try_emplace( /* Key: */ buffer_name,
																						 /* Buffer constructor: */ RHI::BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				RHI::Uniform::BlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, RHI::Uniform::BufferCategory::Intrinsic );

				return &buffer;
			}
		}

		UNREACHABLE();
	}
}
