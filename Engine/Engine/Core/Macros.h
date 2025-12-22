#pragma once

// std Includes.
#include <string_view>

// Typedefs:
#define internal_function static
#define local_persist     static
/* This is used for static global variables, with internal linkage.
 * For global variables intended to be shared across translation units, simply use the extern keyword and define it inside a translation unit. */
#define global_variable	  static
#define header_function   inline

/*
 * Default Constructors:
 */

#define DEFAULT_COPY_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= default;\
	class_name& operator =( const class_name& ) = default;

#define DEFAULT_MOVE_CONSTRUCTORS( class_name )\
	class_name( class_name&& )			   = default;\
	class_name& operator =( class_name&& ) = default;

#define DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= default;\
	class_name& operator =( const class_name& ) = default;\
	class_name( class_name&& )					= default;\
	class_name& operator =( class_name&& )		= default;

#define CONSTEXPR_DEFAULT_COPY_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = default;\
	constexpr class_name& operator =( const class_name& ) = default;

#define CONSTEXPR_DEFAULT_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( class_name&& )			 = default;\
	constexpr class_name& operator =( class_name&& ) = default;

#define CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = default;\
	constexpr class_name& operator =( const class_name& ) = default;\
	constexpr class_name( class_name&& )				  = default;\
	constexpr class_name& operator =( class_name&& )	  = default;

/*
 * Delete Constructors:
 */

#define DELETE_COPY_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= delete;\
	class_name& operator =( const class_name& ) = delete;

#define DELETE_MOVE_CONSTRUCTORS( class_name )\
	class_name( class_name&& )			   = delete;\
	class_name& operator =( class_name&& ) = delete;

#define DELETE_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	class_name( const class_name& )				= delete;\
	class_name& operator =( const class_name& ) = delete;\
	class_name( class_name&& )					= delete;\
	class_name& operator =( class_name&& )		= delete;

#define CONSTEXPR_DELETE_COPY_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = delete;\
	constexpr class_name& operator =( const class_name& ) = delete;

#define CONSTEXPR_DELETE_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( class_name&& )			 = delete;\
	constexpr class_name& operator =( class_name&& ) = delete;

#define CONSTEXPR_DELETE_COPY_AND_MOVE_CONSTRUCTORS( class_name )\
	constexpr class_name( const class_name& )			  = delete;\
	constexpr class_name& operator =( const class_name& ) = delete;\
	constexpr class_name( class_name&& )				  = delete;\
	constexpr class_name& operator =( class_name&& )	  = delete;

/*
 * Comparison Operators:
 */

#define DEFAULT_EQUALITY_OPERATORS( class_name )\
	bool operator ==( const class_name& ) const = default;\
	bool operator !=( const class_name& ) const = default;

#define CONSTEXPR_DEFAULT_EQUALITY_OPERATORS( class_name )\
	constexpr bool operator ==( const class_name& ) const = default;\
	constexpr bool operator !=( const class_name& ) const = default;

/* 
 *Utilities: 
 */

/* Similar to __FILE__, compile-time way of retrieving the current directory of the source file. */
#define __DIR__ ( []() -> std::string_view \
{ \
    constexpr std::string_view path = __FILE__; \
    constexpr size_t pos = path.find_last_of( "/\\") ; \
    return ( pos == std::string_view::npos ) ? "" : path.substr( 0, pos ); \
}() )

#define __DIR_WITH_SEPARATOR__ ( []() -> std::string_view \
{ \
    constexpr std::string_view path = __FILE__; \
    constexpr size_t pos = path.find_last_of( "/\\" ); \
    return ( pos == std::string_view::npos ) ? "" : path.substr( 0, pos + 1 ); \
}() )
