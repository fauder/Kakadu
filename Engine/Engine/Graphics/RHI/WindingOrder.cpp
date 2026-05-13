#include "RHI.h"
#include "WindingOrder.h"
#include "Core/Assertion.h"

namespace Kakadu::RHI
{
	u32 WindingOrderToGLEnum( WindingOrder order )
	{
		switch( order )
		{
			case WindingOrder::Clockwise:        return GL_CW;
			case WindingOrder::CounterClockwise: return GL_CCW;
		}

		ASSERT( false && "Invalid order in Kakadu::RHI::WindingOrderToGLEnum( order )!" );
		return GL_NONE;
	}
}