#pragma once

#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Polar.h"
#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"

#define DEFINE_MATH_TYPES() \
using Degrees = Engine::Math::Degrees< float >; \
using Radians = Engine::Math::Radians< float >; \
\
using DegreesD = Engine::Math::Degrees< double >; \
using RadiansD = Engine::Math::Radians< double >; \
\
using Vector2  = Engine::Math::Vector< float,			2 >; \
using Vector3  = Engine::Math::Vector< float,			3 >; \
using Vector4  = Engine::Math::Vector< float,			4 >; \
using Vector2D = Engine::Math::Vector< double,			2 >; \
using Vector3D = Engine::Math::Vector< double,			3 >; \
using Vector4D = Engine::Math::Vector< double,			4 >; \
using Vector2I = Engine::Math::Vector< int,				2 >; \
using Vector3I = Engine::Math::Vector< int,				3 >; \
using Vector4I = Engine::Math::Vector< int,				4 >; \
using Vector2U = Engine::Math::Vector< unsigned int,	2 >; \
using Vector3U = Engine::Math::Vector< unsigned int,	3 >; \
using Vector4U = Engine::Math::Vector< unsigned int,	4 >; \
\
using Matrix2x2  = Engine::Math::Matrix< float,		2, 2 >; \
using Matrix3x3  = Engine::Math::Matrix< float,		3, 3 >; \
using Matrix4x4  = Engine::Math::Matrix< float,		4, 4 >; \
using Matrix2x2D = Engine::Math::Matrix< double,	2, 2 >; \
using Matrix3x3D = Engine::Math::Matrix< double,	3, 3 >; \
using Matrix4x4D = Engine::Math::Matrix< double,	4, 4 >; \
using Matrix2x2I = Engine::Math::Matrix< int,		2, 2 >; \
using Matrix3x3I = Engine::Math::Matrix< int,		3, 3 >; \
using Matrix4x4I = Engine::Math::Matrix< int,		4, 4 >; \
\
using Matrix2x3   = Engine::Math::Matrix< float,	2, 3 >; \
using Matrix2x4   = Engine::Math::Matrix< float,	2, 4 >; \
using Matrix3x2   = Engine::Math::Matrix< float,	3, 2 >; \
using Matrix3x4   = Engine::Math::Matrix< float,	3, 4 >; \
using Matrix4x2   = Engine::Math::Matrix< float,	4, 2 >; \
using Matrix4x3   = Engine::Math::Matrix< float,	4, 3 >; \
using Matrix2x3D  = Engine::Math::Matrix< double,	2, 3 >; \
using Matrix2x4D  = Engine::Math::Matrix< double,	2, 4 >; \
using Matrix3x2D  = Engine::Math::Matrix< double,	3, 2 >; \
using Matrix3x4D  = Engine::Math::Matrix< double,	3, 4 >; \
using Matrix4x2D  = Engine::Math::Matrix< double,	4, 2 >; \
using Matrix4x3D  = Engine::Math::Matrix< double,	4, 3 >; \
using Matrix2x3I  = Engine::Math::Matrix< int,		2, 3 >; \
using Matrix2x4I  = Engine::Math::Matrix< int,		2, 4 >; \
using Matrix3x2I  = Engine::Math::Matrix< int,		3, 2 >; \
using Matrix3x4I  = Engine::Math::Matrix< int,		3, 4 >; \
using Matrix4x2I  = Engine::Math::Matrix< int,		4, 2 >; \
using Matrix4x3I  = Engine::Math::Matrix< int,		4, 3 >; \
\
using Quaternion  = Engine::Math::Quaternion< float  >; \
using QuaternionD = Engine::Math::Quaternion< double >;
