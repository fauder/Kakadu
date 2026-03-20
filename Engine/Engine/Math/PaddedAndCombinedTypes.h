#pragma once

// Engine Includes.
#include "Color.hpp"
#include "Vector.hpp"
#include "Core/Types.h"

namespace Kakadu::Math
{
	using Color3_Padded = Color4;
	
	struct Color3_AndScalar
	{
		Color3 color;
		float scalar;
	};

	using Vector3_Padded  = Vector4;
	using Vector3D_Padded = Vector4D;
	using Vector3I_Padded = Vector4I;
	using Vector3U_Padded = Vector4U;

	struct Vector3_AndScalar
	{
		Vector3 vector;
		float scalar;
	};

	struct Vector3D_AndScalar
	{
		Vector3D vector;
		double scalar;
	};

	struct Vector3U_AndScalar
	{
		Vector3U vector;
		u32 scalar;
	};

	struct Vector3I_AndScalar
	{
		Vector3I vector;
		i32 scalar;
	};
}
