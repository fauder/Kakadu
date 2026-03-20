#pragma once

// Engine Includes.
#include "Graphics/RHI/Std140StructTag.h"
#include "Math/Color.hpp"
#include "Math/PaddedAndCombinedTypes.h"
#include "Math/Vector.hpp"

namespace Kakadu::Lighting
{
	struct DirectionalLightData : public RHI::Std140StructTag
	{
		Math::Color3_Padded ambient, diffuse, specular;
		Math::Vector3_Padded direction_view_space;
	};

	struct PointLightData : public RHI::Std140StructTag
	{
		Math::Color3_AndScalar ambient_and_attenuation_constant, diffuse_and_attenuation_linear, specular_attenuation_quadratic;
		Math::Vector3_Padded position_view_space;
	};

	struct SpotLightData : public RHI::Std140StructTag
	{
		Math::Color3_Padded ambient, diffuse, specular;
		Math::Vector3_AndScalar position_view_space_and_cos_cutoff_angle_inner, direction_view_space_and_cos_cutoff_angle_outer; // Expected to be in view space when passed to shaders.

	/* End of GLSL correspondance. C++ only extras: */

		Degrees cutoff_angle_inner, cutoff_angle_outer; // Easier to work with angles on the cpu/editor side.
	};
}
