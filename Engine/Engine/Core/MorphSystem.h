#pragma once

// Engine Includes.
#include "Macros.h"
#include "Morph.h"

// std Includes.
#include <vector>

namespace Engine
{
	class MorphSystem
	{
	public:
		using CallableType = Morph::CallableType;

		MorphSystem() = default;
		~MorphSystem() = default;

		DELETE_COPY_AND_MOVE_CONSTRUCTORS( MorphSystem );

		/*
		 * Usage: 
		 */

		void Add( Morph&& new_morph );
		void Execute( const float delta_time_in_seconds );

	private:
		std::vector< Morph > morph_array;
	};
}
