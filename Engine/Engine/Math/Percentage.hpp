#pragma once

// Engine Includes.
#include "Math/Unit.hpp"
#include "Math/Concepts_Math.h"

namespace Engine
{
    namespace Math
    {
        template< std::floating_point FloatType >
        class Percentage : public Unit< FloatType, Percentage >
        {
            using Base = Unit< FloatType, Percentage >;

        public:
            /* Constructors. */
            constexpr Percentage() : Base() {}
            constexpr Percentage( const Percentage& other ) : Base( other ) {}

            explicit Percentage( Initialization::NoInitialization ) : Base( NO_INITIALIZATION ) {}

            /* Value is already normalized: 1.0 == 100% */
            constexpr explicit Percentage( FloatType normalized_value )
                : Base( normalized_value ) {}

            /* Construct (convert) from other underlying type. */
            template< typename OtherType >
            constexpr Percentage( const Unit< OtherType, Percentage >& other )
                : Base( other )
            {}
        };

        namespace Literals
        {
            constexpr Percentage< float > operator"" _percent( long double value )
            {
                return Percentage< float >( static_cast< float >( value * 0.01L ) );
            }

            constexpr Percentage< float > operator"" _percent( unsigned long long value )
            {
                return Percentage< float >( static_cast< float >( value ) * 0.01f );
            }

            constexpr Percentage< double > operator"" _percentd( long double value )
            {
                return Percentage< double >( static_cast< double >( value * 0.01L ) );
            }

            constexpr Percentage< double > operator"" _percentd( unsigned long long value )
            {
                return Percentage< double >( static_cast< double >( value ) * 0.01 );
            }
        }
    }

    using Percentage  = Math::Percentage< float >;
    using PercentageD = Math::Percentage< double >;
}
