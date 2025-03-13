#pragma once

// Engine Includes.
#include "Graphics.h"

// std Includes.
#include <cstdint>

namespace Engine
{
	enum class ClearTarget : unsigned int
	{
		None = 0,

		DepthBuffer   = GL_DEPTH_BUFFER_BIT,
		StencilBuffer = GL_STENCIL_BUFFER_BIT,
		ColorBuffer   = GL_COLOR_BUFFER_BIT,

		All = DepthBuffer | StencilBuffer | ColorBuffer
	};

	enum class PolygonMode : std::uint8_t
	{
		Point,
		Line, 
		Fill,
			
		Wireframe = Line
	};

	enum class ComparisonFunction
	{
		Always	       = GL_ALWAYS,
		Never 	       = GL_NEVER,
		Equal 	       = GL_EQUAL,
		NotEqual       = GL_NOTEQUAL,
		Less 	       = GL_LESS,
		LessOrEqual    = GL_LEQUAL,
		Greater        = GL_GREATER,
		GreaterOrEqual = GL_GEQUAL
	};

	enum class StencilTestResponse
	{
		Keep          = GL_KEEP,
		Zero          = GL_ZERO,
		Replace       = GL_REPLACE,
		Increment     = GL_INCR,
		IncrementWrap = GL_INCR_WRAP,
		Decrement     = GL_DECR,
		DecrementWrap = GL_DECR_WRAP,
		Invert        = GL_INVERT
	};

	enum class BlendingFactor
	{
		Zero                       = GL_ZERO,
		One                        = GL_ONE,
		SourceColor                = GL_SRC_COLOR,
		OneMinusSourceColor        = GL_ONE_MINUS_SRC_COLOR,
		SourceAlpha                = GL_SRC_ALPHA,
		OneMinusSourceAlpha        = GL_ONE_MINUS_SRC_ALPHA,
		DestinationAlpha           = GL_DST_ALPHA,
		OneMinusDestinationAlpha   = GL_ONE_MINUS_DST_ALPHA,
		DestinationColor           = GL_DST_COLOR,
		OneMinusDestinationColor   = GL_ONE_MINUS_DST_COLOR,
		SourceAlphaSaturate        = GL_SRC_ALPHA_SATURATE
	};

	enum class BlendingFunction
	{
		Add             = GL_FUNC_ADD,
		ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
		Subtract        = GL_FUNC_SUBTRACT,
		Minimum         = GL_MIN,
		Maximum         = GL_MAX
	};

	enum class SortingMode : std::uint8_t
	{
		None,
		FrontToBack,
		BackToFront
	};

	enum class Face
	{
		Front        = GL_FRONT,
		Back         = GL_BACK,
		FrontAndBack = GL_FRONT_AND_BACK
	};

	enum class WindingOrder
	{
		Clockwise        = GL_CW,
		CounterClockwise = GL_CCW
	};

#ifdef _EDITOR
	enum class EditorShadingMode : std::uint8_t
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
#endif // _EDITOR
}
