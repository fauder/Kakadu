#pragma once

// Engine Includes.
#include "Math/Concepts.h"
#include "Math/Vector.hpp"

namespace Engine
{
	class Color3 : public Vector3
	{
		using Base = Vector3;

	public:
		constexpr Color3()
			:
			Base( ZERO_INITIALIZATION )
		{}

		constexpr Color3( const unsigned int r, const unsigned int g, const unsigned int b )
			:
			Base( float( r ) / 255.0f, float( g ) / 255.0f, float( b ) / 255.0f )
		{}

		constexpr Color3( const float r, const float g, const float b )
			:
			Base( r, g, b )
		{}

		template< std::size_t VectorSize > requires( VectorSize <= Base::Dimension() )
		constexpr Color3( const Vector< float, VectorSize >& vector )
			:
			Base( vector )
		{}

		constexpr float R() const { return data[ 0 ]; };
		constexpr float G() const { return data[ 1 ]; };
		constexpr float B() const { return data[ 2 ]; };

		constexpr float& R() { return data[ 0 ]; };
		constexpr float& G() { return data[ 1 ]; };
		constexpr float& B() { return data[ 2 ]; };

		static constexpr Color3 Gray( float intensity = 0.55f )		{ return Color3( intensity, intensity, intensity ); }
		static constexpr Color3 White()								{ return Color3(  1.0f,  1.0f,  1.0f ); }
		static constexpr Color3 Black()								{ return Color3(  0.0f,  0.0f,  0.0f ); }
		static constexpr Color3 Red()								{ return Color3(  1.0f,  0.0f,  0.0f ); }
		static constexpr Color3 Green()								{ return Color3(  0.0f,  1.0f,  0.0f ); }
		static constexpr Color3 Blue()								{ return Color3(  0.0f,  0.0f,  1.0f ); }
		static constexpr Color3 Orange()							{ return Color3(  1.0f,  0.65f, 0.0f ); }
		static constexpr Color3 Teal()								{ return Color3(  0.0f,  0.5f,  0.5f ); }
		static constexpr Color3 Cyan()								{ return Color3(  0.0f,  1.0f,  1.0f ); }
		static constexpr Color3 Magenta()							{ return Color3(  1.0f,  0.0f,  1.0f ); }
		static constexpr Color3 Yellow()							{ return Color3(  1.0f,  1.0f,  0.0f ); }
		static constexpr Color3 Clear_Default()						{ return Gray(); }

		static constexpr Color3 Zero()								{ return Black(); }
		static constexpr Color3 One()								{ return White(); }
	};

	class Color4 : public Vector4
	{
		using Base = Vector4;
	
	public:
		constexpr Color4()
			:
			Base( ZERO_INITIALIZATION )
		{}

		constexpr Color4( const unsigned int r, const unsigned int g, const unsigned int b, const unsigned int a )
			:
			Base( float( r ) / 255.0f, float( g ) / 255.0f, float( b ) / 255.0f, float( a ) / 255.0f )
		{}

		constexpr Color4( const float r, const float g, const float b, const float a )
			:
			Base( r, g, b, a )
		{}

		constexpr Color4( const Color3& color, const float alpha )
			:
			Base( color.X(), color.Y(), color.Z(), alpha )
		{}

		template< std::size_t VectorSize > requires( VectorSize <= Base::Dimension() )
		constexpr Color4( const Vector< float, VectorSize >& vector )
			:
			Base( vector )
		{}

		constexpr float R() const { return data[ 0 ]; };
		constexpr float G() const { return data[ 1 ]; };
		constexpr float B() const { return data[ 2 ]; };
		constexpr float A() const { return data[ 3 ]; };

		constexpr float& R() { return data[ 0 ]; };
		constexpr float& G() { return data[ 1 ]; };
		constexpr float& B() { return data[ 2 ]; };
		constexpr float& A() { return data[ 3 ]; };

		/* Because Microsoft is a JOKE company, Function name below has to be surrounded by parentheses to instruct preprocessor to not expand it, i.e., treat it as a macro. */
		Color3& ( RGB )() { return reinterpret_cast< Color3& >( *this ); };
		/* Because Microsoft is a JOKE company, Function name below has to be surrounded by parentheses to instruct preprocessor to not expand it, i.e., treat it as a macro. */
		const Color3& ( RGB )() const { return reinterpret_cast< const Color3& >( *this ); };

		static constexpr Color4 Gray( float intensity = 0.55f, float alpha = 1.0f ) { return Color4( intensity, intensity, intensity, alpha ); }
		static constexpr Color4 White( float alpha = 1.0f )							{ return Color4(  1.0f,  1.0f,  1.0f, alpha ); }
		static constexpr Color4 Black( float alpha = 1.0f )							{ return Color4(  0.0f,  0.0f,  0.0f, alpha ); }
		static constexpr Color4 Red( float alpha = 1.0f )							{ return Color4(  1.0f,  0.0f,  0.0f, alpha ); }
		static constexpr Color4 Green( float alpha = 1.0f )							{ return Color4(  0.0f,  1.0f,  0.0f, alpha ); }
		static constexpr Color4 Blue( float alpha = 1.0f )							{ return Color4(  0.0f,  0.0f,  1.0f, alpha ); }
		static constexpr Color4 Orange( float alpha = 1.0f )						{ return Color4(  1.0f,  0.65f, 0.0f, alpha ); }
		static constexpr Color4 Teal( float alpha = 1.0f )							{ return Color4(  0.0f,  0.5f,  0.5f, alpha ); }
		static constexpr Color4 Cyan( float alpha = 1.0f )							{ return Color4(  0.0f,  1.0f,  1.0f, alpha ); }
		static constexpr Color4 Magenta( float alpha = 1.0f )						{ return Color4(  1.0f,  0.0f,  1.0f, alpha ); }
		static constexpr Color4 Yellow( float alpha = 1.0f )						{ return Color4(  1.0f,  1.0f,  0.0f, alpha ); }
		static constexpr Color4 Clear_Default()										{ return Gray(); }

		static constexpr Color4 Zero()												{ return Black( 0.0f ); }
		static constexpr Color4 One()												{ return White(); }
	};
}
