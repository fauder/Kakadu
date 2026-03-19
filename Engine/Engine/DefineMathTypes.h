#pragma once

#include "Math/Angle.hpp"
#include "Math/Matrix.hpp"
#include "Math/Polar.h"
#include "Math/Quaternion.hpp"
#include "Math/Vector.hpp"

#define DEFINE_MATH_TYPES() \
using u8  = Kakadu::u8;  \
using u16 = Kakadu::u16; \
using u32 = Kakadu::u32; \
using u64 = Kakadu::u64; \
\
using Degrees = Kakadu::Math::Degrees< float >; \
using Radians = Kakadu::Math::Radians< float >; \
\
using DegreesD = Kakadu::Math::Degrees< double >; \
using RadiansD = Kakadu::Math::Radians< double >; \
\
using Vector2  = Kakadu::Math::Vector< float,			2 >; \
using Vector3  = Kakadu::Math::Vector< float,			3 >; \
using Vector4  = Kakadu::Math::Vector< float,			4 >; \
using Vector2D = Kakadu::Math::Vector< double,			2 >; \
using Vector3D = Kakadu::Math::Vector< double,			3 >; \
using Vector4D = Kakadu::Math::Vector< double,			4 >; \
using Vector2I = Kakadu::Math::Vector< int,				2 >; \
using Vector3I = Kakadu::Math::Vector< int,				3 >; \
using Vector4I = Kakadu::Math::Vector< int,				4 >; \
using Vector2U = Kakadu::Math::Vector< u32,				2 >; \
using Vector3U = Kakadu::Math::Vector< u32,				3 >; \
using Vector4U = Kakadu::Math::Vector< u32,				4 >; \
\
using Matrix2x2  = Kakadu::Math::Matrix< float,		2, 2 >; \
using Matrix3x3  = Kakadu::Math::Matrix< float,		3, 3 >; \
using Matrix4x4  = Kakadu::Math::Matrix< float,		4, 4 >; \
using Matrix2x2D = Kakadu::Math::Matrix< double,	2, 2 >; \
using Matrix3x3D = Kakadu::Math::Matrix< double,	3, 3 >; \
using Matrix4x4D = Kakadu::Math::Matrix< double,	4, 4 >; \
using Matrix2x2I = Kakadu::Math::Matrix< int,		2, 2 >; \
using Matrix3x3I = Kakadu::Math::Matrix< int,		3, 3 >; \
using Matrix4x4I = Kakadu::Math::Matrix< int,		4, 4 >; \
\
using Matrix2x3   = Kakadu::Math::Matrix< float,	2, 3 >; \
using Matrix2x4   = Kakadu::Math::Matrix< float,	2, 4 >; \
using Matrix3x2   = Kakadu::Math::Matrix< float,	3, 2 >; \
using Matrix3x4   = Kakadu::Math::Matrix< float,	3, 4 >; \
using Matrix4x2   = Kakadu::Math::Matrix< float,	4, 2 >; \
using Matrix4x3   = Kakadu::Math::Matrix< float,	4, 3 >; \
using Matrix2x3D  = Kakadu::Math::Matrix< double,	2, 3 >; \
using Matrix2x4D  = Kakadu::Math::Matrix< double,	2, 4 >; \
using Matrix3x2D  = Kakadu::Math::Matrix< double,	3, 2 >; \
using Matrix3x4D  = Kakadu::Math::Matrix< double,	3, 4 >; \
using Matrix4x2D  = Kakadu::Math::Matrix< double,	4, 2 >; \
using Matrix4x3D  = Kakadu::Math::Matrix< double,	4, 3 >; \
using Matrix2x3I  = Kakadu::Math::Matrix< int,		2, 3 >; \
using Matrix2x4I  = Kakadu::Math::Matrix< int,		2, 4 >; \
using Matrix3x2I  = Kakadu::Math::Matrix< int,		3, 2 >; \
using Matrix3x4I  = Kakadu::Math::Matrix< int,		3, 4 >; \
using Matrix4x2I  = Kakadu::Math::Matrix< int,		4, 2 >; \
using Matrix4x3I  = Kakadu::Math::Matrix< int,		4, 3 >; \
\
using Quaternion  = Kakadu::Math::Quaternion< float  >; \
using QuaternionD = Kakadu::Math::Quaternion< double >;
