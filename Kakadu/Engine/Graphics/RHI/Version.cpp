// Engine Includes
#include "RHI.h"
#include "Version.h"

// std Includes.
#include <iostream>

namespace Kakadu::RHI
{
	void PrintVersionInfoToConsole()
	{
		const auto version = glGetString( GL_VERSION );
		std::cout << version << "\n\n";
	}
}