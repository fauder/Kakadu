#pragma once

// Engine Includes.
#include "Core/Macros.h"
#include "Core/Macros_SpaceshipOperator.h"

namespace Kakadu
{
	template< typename IDType >
	class ID
	{
	public:
		ID() 
			:
			id( 0 )
		{}

		explicit ID( const unsigned int id )
			:
			id( id )
		{}

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( ID );

		~ID()
		{}

	/* Comparison operators: */

		DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( ID );

	/* Queries: */
		
		bool				IsValid() const { return id > 0;	}

		unsigned int		Get()	  const { return id;		}
		const unsigned int* Address() const { return &id;		}
			  unsigned int* Address()		{ return &id;		}

	/* Setters: */
		void Reset() { id = 0; }

	private:
		unsigned int id;
	};
}
