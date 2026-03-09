// std Includes.
#include <cstdint>

namespace Kakadu
{
	enum class ViewportShadingMode : std::uint8_t
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