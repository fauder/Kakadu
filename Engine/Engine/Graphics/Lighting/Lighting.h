#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Graphics/PaddedAndCombinedTypes.h"
#include "Graphics/Std140StructTag.h"
#include "Math/Vector.hpp"

namespace Engine::Lighting
{
	struct DirectionalLightData : public Std140StructTag
	{
		Color3_Padded ambient, diffuse, specular;
		Vector3_Padded direction_view_space;
	};

	struct PointLightData : public Std140StructTag
	{
		Color3_AndScalar ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_attenuation_quadratic;
		Vector3_Padded position_view_space;
	};

	struct SpotLightData : public Std140StructTag
	{
		Color3_Padded ambient, diffuse, specular;
		Vector3_AndScalar position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer; // Expected to be in view space when passed to shaders.

	/* End of GLSL correspondance. C++ only extras: */

		Degrees cutoff_angle_inner, cutoff_angle_outer; // Easier to work with angles on the cpu/editor side.
	};
}
