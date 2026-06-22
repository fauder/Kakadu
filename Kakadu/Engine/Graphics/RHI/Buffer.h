#pragma once

// Engine Includes.
#include "ID/BufferID.h"
#include "Usage.h"
#include "Core/Types.h"

// std Includes.
#include <span>
#include <string>

// TODO: Keep track of buffer ids and debug/inspect them via ImGuiDrawer::Draw() or something.

namespace Kakadu::RHI
{
	/* This is so we can explicitly work with instance buffers in higher level code while it maps down to a vertex buffer behind the scenes. */
	enum struct BufferType : u8
	{
		Invalid = 0,
		Vertex,
		Instance,
		Index,
		Uniform
	};

	struct Buffer
	{
		Buffer();

		/* Only allocates memory.*/
		Buffer( const BufferType type,
				const u32 size,
				const std::string& name = {},
				const Usage usage = Usage::StaticDraw );

		/* Allocates and sets buffer memory. */
		Buffer( const BufferType type,
				const u32 count,
				const std::span< const std::byte > data_span,
				const std::string& name = {},
				const Usage usage = Usage::StaticDraw );

		Buffer( const Buffer& other );
		Buffer& operator=( const Buffer& other );

		/* Allow moving. */
		Buffer( Buffer&& donor );
		Buffer& operator=( Buffer&& donor );

		~Buffer();

	/* Usage: */

		void Bind() const;
		void Upload( const void* data ) const;
		void Upload_Partial( const std::span< const std::byte > data_span, const std::size_t offset_from_buffer_start ) const;

	/* Queries: */

		operator bool() const { return ( bool )id; } // Use the size to implicitly define validness state.

		BufferID id;
		BufferType type;
		/* 3 bytes of padding. */
		std::string name;
		u32 count;
		u32 size;
	};
}
