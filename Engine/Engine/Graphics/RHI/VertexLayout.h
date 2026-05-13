#pragma once

// Engine Includes.
#include "RHI.h"
#include "ShaderTypeInformation.h"
#include "Core/Types.h"
#include "Math/Concepts_Math.h"

// std Includes.
#include <span>
#include <vector>

namespace Kakadu::RHI
{
	struct VertexAttribute
	{
		i32 count;
		GLenum type;
		bool is_instanced;
		u32 location;

		/* Comparison operators. */
		constexpr bool operator ==( const VertexAttribute& other ) const = default;
		constexpr bool operator !=( const VertexAttribute& other ) const = default;

		bool Empty() const { return count == 0; }

		/* Comparison operators. */

		u32 Size() const { return count * RHI::Type::SizeOf( type ); }
	};

	struct VertexInstanceAttribute
	{
		i32 count;
		GLenum type;
		u32 location;

		/* Comparison operators. */
		constexpr bool operator ==( const VertexInstanceAttribute& other ) const = default;
		constexpr bool operator !=( const VertexInstanceAttribute& other ) const = default;

		bool Empty() const { return count == 0; }

		/* Comparison operators. */

		u32 Size() const { return count * RHI::Type::SizeOf( type ); }
	};

	class VertexLayout
	{
	public:
		VertexLayout();

		/* This makes it possible to pass all attribute lists together in Mesh constructor, even though some of them may not be present.
		 * Empty attributes (count == 0) are skipped, so callers can unconditionally pass the full attribute list. */
		VertexLayout( std::span< const VertexAttribute > attribute_counts_and_types );

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( VertexLayout );

		~VertexLayout();

		/* Comparison operators. */
		constexpr bool operator ==( const VertexLayout& ) const = default;
		constexpr bool operator !=( const VertexLayout& ) const = default;

		void Push( const VertexInstanceAttribute& attribute );

		void SetAndEnableAttributes_NonInstanced() const;
		void SetAndEnableAttributes_Instanced() const;

		u32 Stride_Total() const;
		u32 Stride_NonInstanced() const;
		u32 Stride_Instanced() const;
		
		u32 Count() const { return ( u32 )attributes.size(); }

		bool IsCompatibleWith( const VertexLayout& other ) const;

	private:
		/* Currently unused. */
		void Push( const GLenum type, const i32 count, const bool is_instanced = false );


	private:
		std::vector< VertexAttribute > attributes;
	};
}
