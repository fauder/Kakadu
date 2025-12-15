#pragma once

// Engine Includes.
#include "Graphics/Color.hpp"
#include "Graphics/PaddedAndCombinedTypes.h"
#include "Graphics/Std140StructTag.h"

namespace Engine::MaterialData
{
	struct BlinnPhongMaterialData : public Std140StructTag
	{
		Color3 color_diffuse;
		int has_texture_diffuse;

		Color3 color_emission;
		float shininess;
	};

	struct BasicColorMaterialData : public Std140StructTag
	{
		Color3_Padded color;
	};
}
