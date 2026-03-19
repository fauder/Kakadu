// std Includes.
#include <cstdint>

namespace Kakadu
{
	enum class ViewportShadingMode : u8
	{
		Shaded,
		Wireframe,
		ShadedWireframe,

		TextureCoordinates,
		Geometry_Tangents,
		Geometry_Bitangents,
		Geometry_Normals,
		DebugVectors,
		Shading_Normals
	};
}