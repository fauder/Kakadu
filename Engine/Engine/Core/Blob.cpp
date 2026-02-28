// Engine Includes.
#include "Assertion.h"
#include "Blob.hpp"

// std Includes.
#include <stdexcept> // std::logic_error.

namespace Kakadu
{
	Blob::Blob( const std::size_t size )
		:
		bytes( size, std::byte{ 0 } )
	{
	}

	void Blob::Set( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		WriteBytes( value, offset, size );
	}

	void* Blob::Get( const std::size_t offset )
	{
		return ReadBytes( offset );
	}

	const void* Blob::Get( const std::size_t offset ) const
	{
		return ReadBytes( offset );
	}

	void Blob::Allocate( const std::size_t size, const std::byte value )
	{
		bytes.insert( bytes.end(), size, value );
	}

	void Blob::Deallocate( const std::size_t size )
	{
		ASSERT_DEBUG_ONLY( size < bytes.size() && "Attempting to deallocate more bytes than presently allocated." );

		bytes.erase( bytes.end() - size, bytes.end() );
	}

	void Blob::Clear()
	{
		bytes.clear();
	}

/*
 * PRIVATE API
 */

	void Blob::WriteBytes( const std::byte* value, const std::size_t offset, const std::size_t size )
	{
		std::memcpy( bytes.data() + offset, value, size );
	}

	void* Blob::ReadBytes( std::size_t offset )
	{
		return bytes.data() + offset;
	}

	const void* Blob::ReadBytes( std::size_t offset ) const
	{
		return bytes.data() + offset;
	}
}