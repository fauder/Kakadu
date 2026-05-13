#pragma once

namespace Kakadu::Log
{
	enum class Type
	{
		ERROR_, // wingdi.h has ERROR macro...
		WARNING,
		SUCCESS,
		NORMAL,
		GROUP_SEPARATOR,

		COUNT,

		INVALID
	};
}
