#pragma once

// Engine Includes.
#include "Graphics.h"
#include "GraphicsMacros.h"
#include "ShaderTypeInformation.h"
#include "Math/Concepts.h"

// std Includes.
#include <vector>

namespace Engine
{
	struct VertexAttribute
	{
		int count;
		GLenum type;
		bool is_instanced;
		unsigned int location;

		/* Comparison operators. */
		constexpr bool operator ==( const VertexAttribute& other ) const = default;
		constexpr bool operator !=( const VertexAttribute& other ) const = default;

		bool Empty() const { return count == 0; }

		/* Comparison operators. */

		unsigned int Size() const { return count * GL::Type::SizeOf( type ); }
	};

	struct VertexInstanceAttribute
	{
		int count;
		GLenum type;
		unsigned int location;

		/* Comparison operators. */
		constexpr bool operator ==( const VertexInstanceAttribute& other ) const = default;
		constexpr bool operator !=( const VertexInstanceAttribute& other ) const = default;

		bool Empty() const { return count == 0; }

		/* Comparison operators. */

		unsigned int Size() const { return count * GL::Type::SizeOf( type ); }
	};

	class VertexLayout
	{
	public:
		VertexLayout();

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( VertexLayout );

		// This makes it possible to pass all attribute lists together in Mesh constructor, even though some of them may not be present.
		template< typename Collection >
		VertexLayout( Collection&& attribute_counts_and_types );
		// Definition is just below the class definition because this constructor calls Push(), and therefore, has to be defined after the Push() declaration.

		~VertexLayout();

		/* Comparison operators. */
		constexpr bool operator ==( const VertexLayout& ) const = default;
		constexpr bool operator !=( const VertexLayout& ) const = default;

		void Push( const VertexInstanceAttribute& attribute );

		void SetAndEnableAttributes_NonInstanced() const;
		void SetAndEnableAttributes_Instanced() const;

		unsigned int Stride_Total() const;
		unsigned int Stride_NonInstanced() const;
		unsigned int Stride_Instanced() const;
		
		unsigned int Count() const { return ( unsigned int )attributes.size(); }

		bool IsCompatibleWith( const VertexLayout& other ) const;

	private:
		/* Currently unused. */
		void Push( const GLenum type, const int count, const bool is_instanced = false );


	private:
		std::vector< VertexAttribute > attributes;
	};

	// This makes it possible to pass all attribute lists together in Mesh constructor, even though some of them may not be present.
	template< typename Collection >
	VertexLayout::VertexLayout( Collection&& attribute_counts_and_types )
	{
		for( const auto& attribute : attribute_counts_and_types )
			if( !attribute.Empty() )
				attributes.push_back( attribute );
	}
}
