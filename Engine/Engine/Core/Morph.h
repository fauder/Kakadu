#pragma once

// Engine Includes.
#include "Math/Math.hpp"

// std Includes.
#include <functional>

namespace Engine
{
	struct Morph
	{
		using CallableType           = std::function< void( float ) >;
		using OnCompleteCallableType = std::function< void( void ) >;

		bool operator==( const Morph& other ) const
        {
            return
				on_execute.target< void( * )( float ) >() == other.on_execute.target< void( * )( float ) >() &&
				on_complete.target< void( * )( float ) >() == other.on_complete.target< void( * )( float ) >() &&
				is_looping == other.is_looping &&
				Math::IsEqual( duration_in_seconds, other.duration_in_seconds ) &&
				Math::IsEqual( remaining_duration_in_seconds, other.remaining_duration_in_seconds );
        }

        bool operator!=( const Morph& other ) const
        {
            return !( *this == other );
        }

		CallableType			on_execute  = []( float ){};
		OnCompleteCallableType	on_complete = [](){};

		float duration_in_seconds;
		float remaining_duration_in_seconds;
		bool is_looping = false;
	};
}
