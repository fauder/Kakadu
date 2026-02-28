#pragma once

// Engine Includes.
#include "Scene/Transform.h"

namespace Kakadu
{
	template< typename LightDataType >
	struct Light
	{
		// TODO: When Component hierarchy is implemented, make Components below private, as the client code will utilize GetComponent()-like patterns to access the Components below.

		bool is_enabled;

		LightDataType data;
		Transform* transform;
	};
}
