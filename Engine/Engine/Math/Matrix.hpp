#pragma once

// Engine Includes.
#include "Core/Assertion.h"  
#include "Core/Initialization.h"
#include "Math/Concepts.h"
#include "Math/TypeTraits.h"
#include "Math/Vector.hpp"

namespace Engine::Math
{
	/* Row-major. Post-multiplies a row vector to transform it. */
	template< Concepts::Arithmetic_NotBool Type, std::size_t RowSize, std::size_t ColumnSize >
		requires Concepts::NonZero< RowSize > && Concepts::NonZero< ColumnSize >
	struct Matrix
	{
	/* Constructors & Destructors: */

		CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Matrix );

		constexpr ~Matrix() = default;

		/* Initializes to identity matrix. */
		constexpr Matrix()
			:
			data{}
		{
			if constexpr( RowSize == ColumnSize ) // Do not initialize non-square matrices to identity.
				for( auto i = 0; i < RowSize; i++ )
					data[ i ][ i ] = Type( 1 );
		}

		/* Initialize diagonals. */
		constexpr Matrix( const Vector< Type, RowSize >& vector ) requires( RowSize == ColumnSize )
			:
			data{}
		{
			SetDiagonals( vector );
		}

		constexpr explicit Matrix( Initialization::ZeroInitialization )
			:
			data{}
		{}

#pragma warning(disable:26495) // Suppress "variable is uninitialized" warning, as not initializing it is the whole point of this constructor.
		constexpr explicit Matrix( Initialization::NoInitialization )
		{}
#pragma warning(default:26495)

		constexpr explicit Matrix( const Type( &values )[ RowSize ][ ColumnSize ] )
		{
			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSize; j++ )
					data[ i ][ j ] = values[ i ][ j ];
		}
		constexpr explicit Matrix( Type( &&values )[ RowSize ][ ColumnSize ] )
		{
			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSize; j++ )
					data[ i ][ j ] = values[ i ][ j ];
		}

		constexpr explicit Matrix( const Type( &values )[ RowSize * ColumnSize ] ) // 1D array version.
		{
			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSize; j++ )
					data[ i ][ j ] = values[ i * ColumnSize + j ];
		}

		constexpr explicit Matrix( Type( && values )[ RowSize * ColumnSize ] ) // 1D array version.
		{
			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSize; j++ )
					data[ i ][ j ] = values[ i * ColumnSize + j ];
		}

		template< typename... Vectors >
		constexpr Matrix( Vectors... row_vectors ) requires( sizeof...( Vectors ) == RowSize )
			:
			data{}
		{

			// Utilize fold expressions with a lambda to "loop over" the parameter pack.
			int row_index = 0;
			( /* Lambda: */ [ & ]
			{
				SetRow( row_vectors, row_index++ );
			}
			( ), ... );
		}

#pragma warning(disable:26495) // Suppress "variable is uninitialized" warning, as not initializing it is the whole point of this constructor.
		/* Construct from an upper sub-matrix & a vector for the last row.
		 * Sub-matrix & last row_vector can be ANY size smaller than the size of the matrix to be constructed.
		 */
		template< std::size_t SubMatrixSize, std::size_t VectorSize = RowSize - 1 > requires( RowSize == ColumnSize /* Only allow square matrices. */ && SubMatrixSize < RowSize && VectorSize < RowSize )
		constexpr Matrix( const Matrix< Type, SubMatrixSize, SubMatrixSize >& upper_sub_matrix, const Vector< Type, VectorSize >& last_row_vector = Vector< Type, VectorSize >::Zero() )
		{
			/* Initialize the portion covered by the provided upper sub-matrix. */
			for( auto row_index = 0; row_index < SubMatrixSize; row_index++ )
			{
				for( auto column_index = 0; column_index < SubMatrixSize; column_index++ )
					data[ row_index ][ column_index ] = upper_sub_matrix.data[ row_index ][ column_index ];

				/* Set elements not covered by the upper sub-matrix to zero. */
				for( auto column_index = SubMatrixSize; column_index < RowSize; column_index++ )
					data[ row_index ][ column_index ] = Type( 0 );
			}

			/* If there are any rows between the last row-vector & the upper sub-matrix provided, initialize those rows to zero. */
			for( auto row_index = SubMatrixSize; row_index < RowSize; row_index++ )
				for( auto column_index = 0; column_index < RowSize /* Same as ColumnSize. */; column_index++ )
					data[ row_index ][ column_index ] = Type( 0 );

			/* Initialize last row. */
			for( auto index = 0; index < VectorSize; index++ )
				data[ RowSize - 1 ][ index ] = last_row_vector.data[ index ];

			/* Initialize elements of the last row not covered by the provided vector to zero, except for the last element, which is set to 1. */
			for( auto index = VectorSize; index < RowSize /* Same as ColumnSize. */; index++ )
				data[ RowSize - 1 ][ index ] = Type( index == RowSize - 1 );
		}
#pragma warning(default:26495)

	/* Comparison operators. */
		bool operator== ( const Matrix& right_hand_side ) const
		{
			bool result = true;

			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSize; j++ )
					if constexpr( std::is_integral_v< Type > )
						result &= data[ i ][ j ] == right_hand_side.data[ i ][ j ];
					else
						result &= Math::IsEqual( data[ i ][ j ], right_hand_side.data[ i ][ j ] );
			
			return result;
		}

		bool operator!= ( const Matrix& right_hand_side ) const
		{
			return !operator==( right_hand_side );
		}

	/* Getters & Setters. */
		const Type* Data() const { return &data[ 0 ][ 0 ]; };
		constexpr Type* operator[] ( const unsigned int row_index ) { return data[ row_index ]; }
		constexpr const Type* operator[] ( const unsigned int row_index ) const { return data[ row_index ]; }
		
		/* For now, just allow square sub-matrices. */
		template< std::size_t Size >
		constexpr Matrix< Type, Size, Size > SubMatrix() const requires( RowSize == ColumnSize && Size < RowSize /* If Size == RowSize, no need to create a new Matrix.So, do not use <= , use < instead. */ )
		{
			Matrix< Type, Size, Size > sub_matrix( NO_INITIALIZATION );

			for( auto row_index = 0; row_index < Size; row_index++ )
				for( auto column_index = 0; column_index < Size; column_index++ )
					sub_matrix[ row_index ][ column_index ] = data[ row_index ][ column_index ];

			return sub_matrix;
		}

		template< typename... Values >
		constexpr Matrix& Set( Values... values ) requires( sizeof...( Values ) <= RowSize * ColumnSize )
		{
			// Utilize fold expressions with a lambda to "loop over" the parameter pack.
			int column_index = 0;
			int row_index = 0;
			( /* Lambda: */ [&]
				{
					data[ row_index ][ column_index ] = values;
					if( ++column_index == ColumnSize )
					{
						column_index = 0;
						row_index++;
					}
				}
			( ), ... );

			return *this;
		}

		template< std::size_t VectorSize >
		constexpr Matrix& SetDiagonals( const Vector< Type, VectorSize >& vector ) requires( RowSize == ColumnSize && VectorSize <= RowSize )
		{
			for( auto i = 0; i < VectorSize; i++ )
				data[ i ][ i ] = vector.data[ i ];

			return *this;
		}

		template< std::size_t VectorSize = RowSize >
		constexpr Matrix& SetRow( const Vector< Type, VectorSize >& vector, const unsigned int row_index = 0, const unsigned int start_index_inRow = 0 ) requires( VectorSize <= ColumnSize )
		{
			ASSERT_DEBUG_ONLY( row_index < RowSize && "Row index out of bounds." );
			ASSERT_DEBUG_ONLY( start_index_inRow + VectorSize <= ColumnSize && "Given vector does not fit inside the row when starting from start_index_inRow." );

			for( unsigned int i = 0; i < VectorSize; i++ )
				data[ row_index ][ i + start_index_inRow ] = vector.data[ i ];

			return *this;
		}

		template< std::size_t VectorSize = ColumnSize >
		constexpr Matrix& SetColumn( const Vector< Type, VectorSize >& vector, const unsigned int column_index = 0, const unsigned int start_index_inColumn = 0 ) requires( VectorSize <= RowSize )
		{
			ASSERT_DEBUG_ONLY( column_index < ColumnSize && "Column index out of bounds." );
			ASSERT_DEBUG_ONLY( start_index_inColumn + VectorSize <= RowSize && "Given vector does not fit inside the column when starting from start_index_inColumn." );

			for( unsigned int i = 0; i < VectorSize; i++ )
				data[ i + start_index_inColumn ][ column_index ] = vector.data[ i ];

			return *this;
		}

		template< std::size_t VectorSize = RowSize >
		constexpr const Vector< Type, VectorSize >& GetRow( const unsigned int row_index = 0, const unsigned int start_index_inRow = 0 ) const requires( VectorSize <= ColumnSize )
		{
			ASSERT_DEBUG_ONLY( row_index < RowSize && "Row index out of bounds." );
			ASSERT_DEBUG_ONLY( start_index_inRow + VectorSize <= ColumnSize && "Given vector does not fit inside the row when starting from start_index_inRow." );

			return reinterpret_cast< const Vector< Type, VectorSize >& >( *( data[ row_index ] + start_index_inRow ) );
		}

		template< std::size_t VectorSize = RowSize >
		Vector< Type, VectorSize >& GetRow( const unsigned int row_index = 0, const unsigned int start_index_inRow = 0 ) requires( VectorSize <= ColumnSize )
		{
			ASSERT_DEBUG_ONLY( row_index < RowSize && "Row index out of bounds." );
			ASSERT_DEBUG_ONLY( start_index_inRow + VectorSize <= ColumnSize && "Given vector does not fit inside the row when starting from start_index_inRow." );

			return reinterpret_cast< Vector< Type, VectorSize >& >( *( data[ row_index ] + start_index_inRow ) );
		}

		/* Obligatory copy is returned as the data is laid out such that rows are contiguous, not columns. */
		template< std::size_t VectorSize = ColumnSize >
		constexpr Vector< Type, VectorSize > GetColumn( const unsigned int column_index = 0, const unsigned int start_index_inColumn = 0 ) const requires( VectorSize <= RowSize )
		{
			ASSERT_DEBUG_ONLY( column_index < ColumnSize && "Column index out of bounds." );
			ASSERT_DEBUG_ONLY( start_index_inColumn + VectorSize <= RowSize && "Given vector does not fit inside the column when starting from start_index_inColumn." );

			Vector< Type, VectorSize > vector( NO_INITIALIZATION );

			for( unsigned int i = 0; i < VectorSize; i++ )
				vector.data[ i ] = data[ i + start_index_inColumn ][ column_index ];

			return vector;
		}

		template< std::size_t VectorSize >
		constexpr Matrix& SetScaling( const Vector< Type, VectorSize >& vector ) requires( RowSize == ColumnSize && VectorSize <= RowSize - 1 )
		{
			return SetDiagonals( vector );
		}

		template< std::size_t VectorSize >
		constexpr Matrix& SetTranslation( const Vector< Type, VectorSize >& vector ) requires( RowSize == ColumnSize && VectorSize <= RowSize - 1 )
		{
			return SetRow( vector, RowSize - 1 );
		}

	/* Other Queries. */
		constexpr bool IsIdentity() const { return *this == Identity(); }

		static consteval std::size_t RowCount()     { return RowSize; }
		static consteval std::size_t ColumnCount()  { return ColumnSize; }
		static consteval std::size_t ElementCount() { return RowSize * ColumnSize; }

		static consteval Matrix Identity() { return Matrix(); }

	/* Arithmetic Operations. */

		/* Matrix-matrix multiplication. */
		template< std::size_t RowSizeOther, std::size_t ColumnSizeOther >
		constexpr Matrix< Type, RowSize, ColumnSizeOther > operator* ( const Matrix< Type, RowSizeOther, ColumnSizeOther >& other ) const requires( ColumnSize == RowSizeOther )
		{
			Matrix< Type, RowSize, ColumnSizeOther > result( ZERO_INITIALIZATION );
			for( auto i = 0; i < RowSize; i++ )
				for( auto j = 0; j < ColumnSizeOther; j++ )
					for( auto k = 0; k < ColumnSize; k++ )
						result.data[ i ][ j ] += data[ i ][ k ] * other.data[ k ][ j ];

			return result;
		}

		constexpr Matrix& operator*= ( const Matrix& other )
		{
			return *this = Matrix( *this ) * other;
		}

		/* Arithmetic Operations: Unary operators. */
		constexpr Matrix operator- () const
		{
			return *this * Type( -1 );
		}

		Matrix& Transpose()
		{
			for( auto i = 0; i < RowSize; i++ )
			{
				for( auto j = i + 1; j < ColumnSize; j++ )
					std::swap( data[ i ][ j ], data[ j ][ i ] );
			}

			return *this;
		}

		Matrix Transposed() const
		{
			return Matrix( *this ).Transpose();
		}

		constexpr Type Trace() const requires( RowSize == ColumnSize )
		{
			Type result( 0 );
			for( auto index = 0; index < RowSize; index++ )
				result += data[ index ][ index ];

			return result;
		}

		constexpr Type Determinant() const requires( RowSize == ColumnSize )
		{
			if constexpr( RowSize == 2 && ColumnSize == 2 )
				return data[ 0 ][ 0 ] * data[ 1 ][ 1 ] - data[ 1 ][ 0 ] * data[ 0 ][ 1 ];
			if constexpr( RowSize == 3 && ColumnSize == 3 )
				return
					data[ 0 ][ 0 ] * ( ( data[ 1 ][ 1 ] * data[ 2 ][ 2 ] ) - ( data[ 2 ][ 1 ] * data[ 1 ][ 2 ] ) ) -
					data[ 0 ][ 1 ] * ( data[ 1 ][ 0 ] * data[ 2 ][ 2 ] - data[ 2 ][ 0 ] * data[ 1 ][ 2 ] ) +
					data[ 0 ][ 2 ] * ( data[ 1 ][ 0 ] * data[ 2 ][ 1 ] - data[ 2 ][ 0 ] * data[ 1 ][ 1 ] );
		}

		/* Row-major. */
		Type data[ RowSize ][ ColumnSize ];
	};

	/* Vector-matrix multiplication. */
	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
	constexpr Vector< Type, RowSize > operator* ( const Vector< Type, RowSize >& vector, const Matrix< Type, RowSize, ColumnSize >& matrix )
	{
		Vector< Type, RowSize > vector_transformed;
		for( auto j = 0; j < ColumnSize; j++ )
			for( auto k = 0; k < RowSize; k++ )
				vector_transformed[ j ] += vector[ k ] * matrix.data[ k ][ j ];

		return vector_transformed;
	}

	/* Vector-matrix multiplication. */
	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
	constexpr Vector< Type, RowSize >& operator*= ( Vector< Type, RowSize >& vector, const Matrix< Type, RowSize, ColumnSize >& matrix )
	{
		return vector = vector * matrix;
	}

	/* Matrix-vector multiplication: Treating the vector on the right as a column matrix, i.e., a matrix with 1 column and ColumnSize rows.
	 * Produces a row vector of size RowSize. */
	template< Concepts::Arithmetic Type, std::size_t RowSize, std::size_t ColumnSize >
	constexpr Vector< Type, RowSize > operator* ( const Matrix< Type, RowSize, ColumnSize >& matrix, const Vector< Type, ColumnSize >& column_vector )
	{
		Vector< Type, RowSize > vector_transformed;
		for( auto k = 0; k < RowSize; k++ )
			for( auto j = 0; j < ColumnSize; j++ )
				vector_transformed[ k ] += column_vector[ j ] * matrix.data[ k ][ j ];

		return vector_transformed;
	}
}

namespace Engine
{
	using Matrix2x2  = Math::Matrix< float,		2, 2 >;
	using Matrix3x3  = Math::Matrix< float,		3, 3 >;
	using Matrix4x4  = Math::Matrix< float,		4, 4 >;
	using Matrix2x2D = Math::Matrix< double,	2, 2 >;
	using Matrix3x3D = Math::Matrix< double,	3, 3 >;
	using Matrix4x4D = Math::Matrix< double,	4, 4 >;
	using Matrix2x2I = Math::Matrix< int,		2, 2 >;
	using Matrix3x3I = Math::Matrix< int,		3, 3 >;
	using Matrix4x4I = Math::Matrix< int,		4, 4 >;

	using Matrix2x3   = Math::Matrix< float,	2, 3 >;
	using Matrix2x4   = Math::Matrix< float,	2, 4 >;
	using Matrix3x2   = Math::Matrix< float,	3, 2 >;
	using Matrix3x4   = Math::Matrix< float,	3, 4 >;
	using Matrix4x2   = Math::Matrix< float,	4, 2 >;
	using Matrix4x3   = Math::Matrix< float,	4, 3 >;
	using Matrix2x3D  = Math::Matrix< double,	2, 3 >;
	using Matrix2x4D  = Math::Matrix< double,	2, 4 >;
	using Matrix3x2D  = Math::Matrix< double,	3, 2 >;
	using Matrix3x4D  = Math::Matrix< double,	3, 4 >;
	using Matrix4x2D  = Math::Matrix< double,	4, 2 >;
	using Matrix4x3D  = Math::Matrix< double,	4, 3 >;
	using Matrix2x3I  = Math::Matrix< int,		2, 3 >;
	using Matrix2x4I  = Math::Matrix< int,		2, 4 >;
	using Matrix3x2I  = Math::Matrix< int,		3, 2 >;
	using Matrix3x4I  = Math::Matrix< int,		3, 4 >;
	using Matrix4x2I  = Math::Matrix< int,		4, 2 >;
	using Matrix4x3I  = Math::Matrix< int,		4, 3 >;
}
