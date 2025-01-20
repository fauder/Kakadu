#pragma once

// Engince Includes.
#include "Concepts.h"
#include "Vector.hpp"
#include "Graphics/Color.hpp"

// std Includes.
#include <random>

namespace Engine::Math
{
	// Singleton.
	class Random
	{
	public:
		DELETE_COPY_AND_MOVE_CONSTRUCTORS( Random );

		static void Seed( const unsigned int seed )
		{
			Instance( seed );
		}

		static void SeedRandom()
		{
			Instance( std::random_device()( ) );
		}

		template< Concepts::Arithmetic Type >
		static Type Generate( const Type min = Type( 0 ), const Type max = Type( 1 ) )
		{
			auto& instance = Instance();

			if constexpr( std::is_integral_v< Type > )
			{
				std::uniform_int_distribution< Type > uniform_dist( min, max );
				return uniform_dist( instance.engine );
			}
			else
			{
				std::uniform_real_distribution< Type > uniform_dist( min, max );
				return uniform_dist( instance.engine );
			}
		}

		template< Concepts::Angular AngleType >
		static AngleType Generate( const AngleType min = AngleType( 0 ), const AngleType max = AngleType( 1 ) )
		{
			auto& instance = Instance();

			std::uniform_real_distribution< typename AngleType::UnderlyingType > uniform_dist( min.Value(), max.Value() );
			return AngleType( uniform_dist( instance.engine ) );
		}

		template< typename VectorType > requires( VectorType::Dimension() > 1 )
		static VectorType Generate( const VectorType& min = VectorType::Zero(), 
									const VectorType& max = VectorType::One() )
		{
			auto& instance = Instance();

			VectorType vector;

			for( auto i = 0; i < VectorType::Dimension(); i++ )
				vector[ i ] = Generate( min[ i ], max[ i ] );

			return vector;
		}

		template< Concepts::Arithmetic Type, std::size_t Length >
		static std::array< Type, Length > Generate( const Type min = Type( 0 ), const Type max = Type( 1 ) )
		{
			auto& instance = Instance();

			std::array< Type, Length > result;

			for( auto i = 0; i < Length; i++ )
				result[ i ] = Generate( min, max );

			return result;
		}

	private:
		Random( const unsigned int seed )
			:
			engine( seed )
		{}

		static Random& Instance( const unsigned int seed = std::default_random_engine::default_seed )
		{
			static Random instance( seed );
			return instance;
		}

	private:
		std::random_device device;
		std::default_random_engine engine;
	};
}
