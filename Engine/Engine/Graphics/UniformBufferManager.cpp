// Engine Includes.
#include "UniformBufferManager.h"
#include "UniformBlockBindingPointManager.h"
#include "Core/Optimization.h"

namespace Kakadu
{
	Buffer* UniformBufferManager::CreateOrRequest( const std::string& buffer_name, const Uniform::BufferInformation& buffer_info )
	{
		UniformBufferManager& instance = Instance();

		switch( buffer_info.category )
		{
			case Uniform::BufferCategory::Regular:
			{
				Buffer& buffer = instance.uniform_buffer_map_regular.try_emplace( /* Key: */ buffer_name,
																				  BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				UniformBlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Regular );

				return &buffer;
			}
			case Uniform::BufferCategory::Global:
			{
				Buffer& buffer = instance.uniform_buffer_map_global.try_emplace( /* Key: */ buffer_name,
																				 BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				UniformBlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Global );

				return &buffer;
			}
			case Uniform::BufferCategory::Intrinsic:
			{
				Buffer& buffer = instance.uniform_buffer_map_intrinsic.try_emplace( /* Key: */ buffer_name,
																					BufferType::Uniform, buffer_info.size, buffer_name ).first->second;
				UniformBlockBindingPointManager::ConnectBufferToBlock( buffer, buffer_name, Uniform::BufferCategory::Intrinsic );

				return &buffer;
			}
		}

		UNREACHABLE();
	}
}
