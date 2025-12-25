#pragma once

// Engine Includes.
#include "Core/Utility.hpp"
#include "Math/Concepts_Math.h"
#include "Math/Constants.h"
#include "Math/Unit.hpp"

// std Includes.
#include <cmath>

namespace Engine
{
	namespace Math
	{
		template< std::floating_point > class Degrees;
		template< std::floating_point > class Radians;

		template< std::floating_point FloatType >
		class Degrees : public Unit< FloatType, Degrees >
		{
			using Base = Unit< FloatType, Degrees >;

		public:
		/* Constructors. */
			constexpr Degrees() : Base() {}
			constexpr Degrees( const Degrees& other ) : Base( other ) {}
			
			explicit Degrees( Initialization::NoInitialization ) : Base( NO_INITIALIZATION ) {}

			constexpr explicit Degrees( FloatType value ) : Base( value ) {}

			/* Construct (convert) from other underlying type. */
			template< typename OtherType >
			constexpr Degrees( const Unit< OtherType, Degrees >& other ) : Base( other ) {}

			/* Construct from Radians. */
			constexpr Degrees( const Unit< FloatType, Radians >& radians ) : Base( FloatType( radians ) * Constants< FloatType >::Rad_To_Deg() ) {}
			
		/* Trigonometric Functions. */
			/* Wraps a given angle between (-180, +180] range. */
			static Degrees Wrapped180( const Degrees angle )
			{
				if( std::abs( ( FloatType )angle ) > ( FloatType )180.0 )
				{
					constexpr FloatType pi     = FloatType( 180.0 );
					constexpr FloatType two_pi = FloatType( 360.0 );

					FloatType float_angle = angle.Value();

					return Degrees( float_angle - two_pi * std::floor( ( float_angle + pi ) / two_pi ) );
				}

				return angle;
			}

			/* Wraps a given angle between (-180, +180] range. Returns self for daisy-chaining. */
			constexpr Degrees& Wrap180()
			{
				if( std::abs( Base::value ) > ( FloatType )180.0 )
				{
					constexpr FloatType pi     = FloatType( 180.0 );
					constexpr FloatType two_pi = FloatType( 360.0 );

					Base::value -= two_pi * std::floor( ( Base::value + pi ) / two_pi );
				}

				return *this;
			}

		/* String Utility. */
			template< std::size_t Length >
			constexpr static auto Format( const char( &number_format )[ Length ] ) { return Utility::String::ConstexprConcatenate( number_format, " degrees" ); }
		};

		template< std::floating_point FloatType >
		class Radians : public Unit< FloatType, Radians >
		{
			using Base = Unit< FloatType, Radians >;

		public:
		/* Constructors. */
			constexpr Radians() : Base() {}
			constexpr Radians( const Radians& other ) : Base( other ) {}

			explicit Radians( Initialization::NoInitialization ) : Base( NO_INITIALIZATION ) {}

			constexpr explicit Radians( FloatType value ) : Base( value ) {}

			/* Construct (convert) from other underlying type. */
			template< typename OtherType >
			constexpr Radians( const Unit< OtherType, Radians >& other ) : Base( other ) {}

			/* Construct from Degrees. */
			constexpr Radians( const Unit< FloatType, Degrees >& degrees ) : Base( FloatType( degrees ) * Constants< FloatType >::Deg_To_Rad() ) {}
		
		/* Trigonometric Functions. */
			/* Wraps a given angle between (-PI, +PI] range. */
			static Radians WrappedPi( const Radians angle )
			{
				constexpr FloatType pi = Constants< FloatType >::Pi();

				if( std::abs( ( FloatType )angle ) > pi )
				{
					constexpr FloatType two_pi = Constants< FloatType >::Two_Pi();
					
					FloatType float_angle = angle.Value();

					return Radians( float_angle - two_pi * std::floor( ( float_angle + pi ) / two_pi ) );
				}

				return angle;
			}

			/* Wraps a given angle between (-PI, +PI] range. Returns self for daisy-chaining. */
			constexpr Radians& WrapPi()
			{
				constexpr FloatType pi = Constants< FloatType >::Pi();

				if( std::abs( Base::value ) > pi )
				{
					constexpr FloatType two_pi = Constants< FloatType >::Two_Pi();

					Base::value -= two_pi * std::floor( ( Base::value + pi ) / two_pi );
				}

				return *this;
			}

		/* String Utility. */
			template< std::size_t Length >
			constexpr static auto Format( const char( &number_format )[ Length ] ) { return Utility::String::ConstexprConcatenate( number_format, " radians" ); }
		};

		namespace Literals
		{
			constexpr Math::Degrees< float > operator"" _deg( long double value )
			{
				return Math::Degrees< float >( static_cast< float >( value ) );
			}

			constexpr Math::Degrees< float > operator"" _deg( unsigned long long value )
			{
				return Math::Degrees< float >( static_cast< float >( value ) );
			}

			constexpr Math::Degrees< double > operator"" _degd( long double value )
			{
				return Math::Degrees< double >( static_cast< double >( value ) );
			}

			constexpr Math::Degrees< double > operator"" _degd( unsigned long long value )
			{
				return Math::Degrees< double >( static_cast< double >( value ) );
			}

			constexpr Math::Radians< float > operator"" _rad( long double value )
			{
				return Math::Radians< float >( static_cast< float >( value ) );
			}

			constexpr Math::Radians< float > operator"" _rad( unsigned long long value )
			{
				return Math::Radians< float >( static_cast< float >( value ) );
			}

			constexpr Math::Radians< double > operator"" _radd( long double value )
			{
				return Math::Radians< double >( static_cast< double >( value ) );
			}

			constexpr Math::Radians< double > operator"" _radd( unsigned long long value )
			{
				return Math::Radians< double >( static_cast< double >( value ) );
			}
		};
	}

	using Degrees = Math::Degrees< float >;
	using Radians = Math::Radians< float >;

	using DegreesD = Math::Degrees< double >;
	using RadiansD = Math::Radians< double >;

	template<> struct Constants< Degrees >
	{
		Constants() = delete;

		static constexpr Degrees Pi()				{ return Degrees( 180.0f );						}
		static constexpr Degrees Pi_Over_Two()		{ return Pi() / 2.0f;							}
		static constexpr Degrees Pi_Over_Three()	{ return Pi() / 3.0f;							}
		static constexpr Degrees Pi_Over_Four()		{ return Pi() / 4.0f;							}
		static constexpr Degrees Pi_Over_Six()		{ return Pi() / 6.0f;							}
		static constexpr Degrees Two_Pi()			{ return Pi() * 2.0f;							}
	};

	template<> struct Constants< Radians >
	{
		Constants() = delete;

		static constexpr Radians Pi()				{ return Radians( Constants< float >::Pi() );	}
		static constexpr Radians Pi_Over_Two()		{ return Pi() / 2.0f;							}
		static constexpr Radians Pi_Over_Three()	{ return Pi() / 3.0f;							}
		static constexpr Radians Pi_Over_Four()		{ return Pi() / 4.0f;							}
		static constexpr Radians Pi_Over_Six()		{ return Pi() / 6.0f;							}
		static constexpr Radians Two_Pi()			{ return Pi() * 2.0f;							}
	};

	template<> struct Constants< DegreesD >
	{
		Constants() = delete;

		static constexpr DegreesD Pi()				{ return DegreesD( 180.0 );						}
		static constexpr DegreesD Pi_Over_Two()		{ return Pi() / 2.0;							}
		static constexpr DegreesD Pi_Over_Three()	{ return Pi() / 3.0;							}
		static constexpr DegreesD Pi_Over_Four()	{ return Pi() / 4.0;							}
		static constexpr DegreesD Pi_Over_Six()		{ return Pi() / 6.0;							}
		static constexpr DegreesD Two_Pi()			{ return Pi() * 2.0;							}
	};

	template<> struct Constants< RadiansD >
	{
		Constants() = delete;

		static constexpr RadiansD Pi()				{ return RadiansD( Constants< double >::Pi() );	}
		static constexpr RadiansD Pi_Over_Two()		{ return Pi() / 2.0;							}
		static constexpr RadiansD Pi_Over_Three()	{ return Pi() / 3.0;							}
		static constexpr RadiansD Pi_Over_Four()	{ return Pi() / 4.0;							}
		static constexpr RadiansD Pi_Over_Six()		{ return Pi() / 6.0;							}
		static constexpr RadiansD Two_Pi()			{ return Pi() * 2.0;							}
	};

	namespace Concepts
	{
		template< typename T > concept Angular = std::is_same_v< T, Degrees > || std::is_same_v< T, DegreesD > || std::is_same_v< T, Radians > || std::is_same_v< T, RadiansD >;
	}
}
