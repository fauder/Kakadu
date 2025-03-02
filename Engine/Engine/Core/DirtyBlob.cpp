// Engine Includes.
#include "DirtyBlob.h"

// std Includes.
#include <algorithm>

namespace Engine
{
	DirtyBlob::DirtyBlob( const std::size_t size )
		:
		Blob( size )
	{
	}

	void DirtyBlob::Set( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		Blob::Set( value, offset, size );

		dirty_sections.emplace_back( offset, size );
	}

	void DirtyBlob::Clear()
	{
		Blob::Clear();

		dirty_sections.clear();
	}

	void DirtyBlob::MergeConsecutiveDirtySections()
	{
		std::sort( dirty_sections.begin(), dirty_sections.end() );

		for( int index = 0; index < dirty_sections.size() - 1; index++ )
		{
			auto& section = dirty_sections[ index ];
			auto& next_section = dirty_sections[ index + 1 ];

			if( ( section.offset + section.size ) == next_section.offset )
			{
				// Remove the first one (rather; mark for removal) & grow the next one to include both. This way, as the loop iterates forward, merging > 2 sections is possible.
				next_section.offset = section.offset;
				next_section.size  += section.size;

				section.offset = -1; // Mark the section for removal.
			}
		}

		dirty_sections.erase( std::remove_if( dirty_sections.begin(), dirty_sections.end(), []( const Section& section ) { return section.offset == -1; } ),
							  dirty_sections.end() );
	}

	void DirtyBlob::ClearDirtySections()
	{
		dirty_sections.clear();
	}

	std::span< std::byte > DirtyBlob::SpanFromSection( const Section& section )
	{
		return std::span< std::byte >( bytes.begin() + section.offset, section.size );
	}
}