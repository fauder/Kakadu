#pragma once

// Engine Includes.
#include "Math/Color.hpp"
#include "Math/PaddedAndCombinedTypes.h"
#include "Graphics/RHI/Std140StructTag.h"

namespace Kakadu::MaterialData
{
	struct BlinnPhongMaterialData : public RHI::Std140StructTag
	{
		Color3 color_diffuse;
		i32 has_texture_diffuse;

		Color3 color_emission;
		float shininess;
	};

	struct BasicColorMaterialData : public RHI::Std140StructTag
	{
		Math::Color3_Padded color;
	};
}
