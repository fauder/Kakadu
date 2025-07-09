// Engine Includes.
#include "MorphSystem.h"

// std Includes.
#include <stdexcept>
namespace Engine
{
	void MorphSystem::Add( Morph&& new_morph )
	{
#ifdef _EDITOR
		if( not new_morph.on_execute )
		{
			throw std::runtime_error( "MorphSystem::Add(): Attempt to Add() Morph with unset on_execute callback detected!" );
		}
#endif // _EDITOR

		new_morph.remaining_duration_in_seconds = new_morph.duration_in_seconds;

		morph_array.emplace_back( std::move( new_morph ) );
	}

	void MorphSystem::Execute( const float delta_time_in_seconds )
	{
		// Use erase-remove idiom to remove finished Morphs.
		auto remove_iterator = morph_array.end();

		for( auto& morph : morph_array )
		{
			if( morph.remaining_duration_in_seconds < delta_time_in_seconds )
			{
				const auto t = ( morph.duration_in_seconds - morph.remaining_duration_in_seconds ) / morph.duration_in_seconds;

				morph.on_execute( t );
				if( morph.on_complete )
					morph.on_complete();

				remove_iterator = std::remove( morph_array.begin(), morph_array.end(), morph );
			}
			else
			{
				const auto t = ( morph.duration_in_seconds - ( morph.remaining_duration_in_seconds - delta_time_in_seconds ) ) / morph.duration_in_seconds;

				morph.on_execute( t );

				morph.remaining_duration_in_seconds -= delta_time_in_seconds;
			}
		}

		if( remove_iterator != morph_array.end() )
			morph_array.erase( remove_iterator, morph_array.end() );
	}
}