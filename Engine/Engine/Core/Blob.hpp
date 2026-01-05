#pragma once

// Engine Includes.
#include "Macros.h"

// std Includes.
#include <cstddef> // std::byte.
#include <vector>

namespace Engine
{
	class Blob
	{
	public:
		Blob() = default;
		Blob( const std::size_t size );

		DELETE_COPY_CONSTRUCTORS( Blob );
		DEFAULT_MOVE_CONSTRUCTORS( Blob );

		~Blob() = default;

	/* Set/Get: */
		template< typename Type >
		void Set( const Type& value, const std::size_t offset )
		{
			WriteBytes( ( std::byte* )( &value ), offset, sizeof( Type ) );
		}

		void Set( const std::byte* value, const std::size_t offset, const std::size_t size );
		void* Get( const std::size_t offset );
		const void* Get( const std::size_t offset ) const;

		template< typename Type >
		Type& Get( const std::size_t offset )
		{
			auto pointer = ReadBytes( offset );
			return reinterpret_cast< Type& >( *( ( Type* )pointer ) );
		}

		template< typename Type >
		const Type& Get( const std::size_t offset ) const
		{
			const auto pointer = ReadBytes( offset );
			return reinterpret_cast< const Type& >( *( ( const Type* )pointer ) );
		}

		std::size_t CurrentSize() const { return bytes.size(); }

	/* Allocation/Deallocation: */
		void Allocate( const std::size_t size, const std::byte value = std::byte{ 0 } );
		void Deallocate( const std::size_t size );
		void Clear();

	private:
		void WriteBytes( const std::byte* value, const std::size_t offset, const std::size_t size );
		void* ReadBytes( std::size_t offset );
		const void* ReadBytes( std::size_t offset ) const;

	protected:
		std::vector< std::byte > bytes;
	};
}
