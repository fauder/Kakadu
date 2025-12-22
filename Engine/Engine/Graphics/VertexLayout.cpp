// Engine Includes.
#include "VertexLayout.hpp"

#include "Asset/Shader/_Attributes.glsl"

namespace Engine
{
	VertexLayout::VertexLayout()
	{}

	VertexLayout::~VertexLayout()
	{}

	void VertexLayout::Push( const VertexInstanceAttribute& attribute )
	{
		// Assume that At least 1 non-instanced attribute (which is position) is present. This way, no "if( attributes.empty() )" check are necessary.
		ASSERT_DEBUG_ONLY( not attributes.empty() && "No non-instanced attributes found before attempting to push instanced attribute!" );

		ASSERT_DEBUG_ONLY( attribute.location >= INSTANCED_ATTRIBUTE_START && "Instanced attributes can not have locations smaller than the INSTANCED_ATTRIBUTE_START value!" );

		/*const auto& previous_attribute = attributes.back();
		const auto& [ previous_attribute_occupied_slot_count, dont_care_slot_size ] = GL::Type::RowAndColumnCountOf( previous_attribute.type );*/
		attributes.push_back( {
			.count        = attribute.count,
			.type         = attribute.type,
			.is_instanced = true,
			.location     = attribute.location
			//.location     = s_attribute.location + previous_attribute_occupied_slot_count
		} );
	}

	void VertexLayout::SetAndEnableAttributes_NonInstanced() const
	{
		const auto instanced_attributes_begin = std::find_if( attributes.cbegin(), attributes.cend(), []( const VertexAttribute& attribute ) { return attribute.is_instanced; } );

		const unsigned int stride = Stride_NonInstanced();

		constexpr int is_normalized = GL_FALSE;
		unsigned int offset = 0;

		for( auto iterator = attributes.cbegin(); iterator != instanced_attributes_begin; iterator++ )
		{
			const auto& attribute = *iterator;

			if( const auto underlying_count = GL::Type::CountOf( attribute.type );
				underlying_count > 1 )
			{
				const auto underlying_type            = GL::Type::ComponentTypeOf( attribute.type );
				const auto underlying_type_size       = GL::Type::SizeOf( underlying_type );
				const auto& [ slot_count, slot_size ] = GL::Type::RowAndColumnCountOf( attribute.type );
				const auto slot_stride                = slot_size * underlying_type_size;
				
				ASSERT_DEBUG_ONLY( underlying_type != GL_DOUBLE ); // DOUBLES ARE NOT IMPLEMENTED FOR CONVENIENCE.

				for( auto slot_index = 0; slot_index < slot_count; slot_index++ )
				{
					const auto location = attribute.location + slot_index;
					glVertexAttribPointer( location, slot_size, underlying_type, is_normalized, stride, BUFFER_OFFSET( offset ) );
					glEnableVertexAttribArray( location );

					offset += slot_stride;
				}
			}
			else
			{
				glVertexAttribPointer( attribute.location, attribute.count, attribute.type, is_normalized, stride, BUFFER_OFFSET( offset ) );
				glEnableVertexAttribArray( attribute.location );

				offset += attribute.Size();
			}
		}
	}

	void VertexLayout::SetAndEnableAttributes_Instanced() const
	{
		const auto instanced_attributes_begin = std::find_if( attributes.cbegin(), attributes.cend(), []( const VertexAttribute& attribute ) { return attribute.is_instanced; } );

		const unsigned int stride = Stride_Instanced();

		constexpr int is_normalized = GL_FALSE;
		unsigned int offset = 0;

		for( auto iterator = instanced_attributes_begin; iterator != attributes.cend(); iterator++ )
		{
			const auto& attribute = *iterator;

			if( const auto underlying_count = GL::Type::CountOf( attribute.type );
				underlying_count > 1 )
			{
				const auto underlying_type            = GL::Type::ComponentTypeOf( attribute.type );
				const auto underlying_type_size       = GL::Type::SizeOf( underlying_type );
				const auto& [ slot_count, slot_size ] = GL::Type::RowAndColumnCountOf( attribute.type );
				const auto slot_stride                = slot_size * underlying_type_size;
				
				ASSERT_DEBUG_ONLY( underlying_type != GL_DOUBLE ); // DOUBLES ARE NOT IMPLEMENTED FOR CONVENIENCE.

				for( auto slot_index = 0; slot_index < slot_count; slot_index++ )
				{
					const auto location = attribute.location + slot_index;
					glVertexAttribPointer( location, slot_size, underlying_type, is_normalized, stride, BUFFER_OFFSET( offset ) );
					glEnableVertexAttribArray( location );

					// Instancing:
					glVertexAttribDivisor( location, 1 );

					offset += slot_stride;
				}
			}
			else
			{
				glVertexAttribPointer( attribute.location, attribute.count, attribute.type, is_normalized, stride, BUFFER_OFFSET( offset ) );
				glEnableVertexAttribArray( attribute.location );

				// Instancing:
				glVertexAttribDivisor( attribute.location, 1 );

				offset += attribute.Size();
			}
		}
	}

	unsigned int VertexLayout::Stride_Total() const
	{
		unsigned int stride = 0;
		for( auto index = 0; index < attributes.size(); index++ )
			stride += attributes[ index ].Size();

		return stride;
	}

	unsigned int VertexLayout::Stride_NonInstanced() const
	{
		unsigned int stride = 0;
		for( auto index = 0; index < attributes.size(); index++ )
			stride += attributes[ index ].Size() * ( 1 - attributes[ index ].is_instanced );

		return stride;
	}

	unsigned int VertexLayout::Stride_Instanced() const
	{
		unsigned int stride = 0;
		for( auto index = 0; index < attributes.size(); index++ )
			stride += attributes[ index ].Size() * attributes[ index ].is_instanced;

		return stride;
	}

	bool VertexLayout::IsCompatibleWith( const VertexLayout& other ) const
	{
		if( other.Count() != Count() )
			return false;

		for( auto i = 0; i < attributes.size(); i++ )
			if( other.attributes[ i ] != attributes[ i ] )
				return false;
			
		return true;
	}

	/*
	 * Private API
	 */

	/* Currently unused. */
	void VertexLayout::Push( const GLenum type, const int count, const bool is_instanced )
	{
		if( attributes.empty() )
			attributes.push_back( { count, type, is_instanced, 0 } );
		else
		{
			const auto& previous_attribute = attributes.back();
			const auto& [previous_attribute_occupied_slot_count, dont_care_slot_size] = GL::Type::RowAndColumnCountOf( previous_attribute.type );
			attributes.push_back(
			{
				.count        = count,
				.type         = type,
				.is_instanced = is_instanced,
				.location     = previous_attribute.location + previous_attribute_occupied_slot_count
			} );
		}
	}
}
