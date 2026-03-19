#pragma once
// Engine Includes.
#include "Assertion.h"

#ifdef _DEBUG
    #ifdef _MSC_VER
        #define UNREACHABLE() ASSERT( "SHOULD HAVE BEEN UNREACHABLE." ); __assume( 0 )
    #elif defined( __GNUC__ ) || defined( __clang__ )
        #define UNREACHABLE() ASSERT( "SHOULD HAVE BEEN UNREACHABLE." ); __builtin_unreachable()
    #else
        #define UNREACHABLE() ASSERT( "SHOULD HAVE BEEN UNREACHABLE." )
    #endif
#else
    #ifdef _MSC_VER
        #define UNREACHABLE() __assume( 0 )
    #elif defined( __GNUC__ ) || defined( __clang__ )
        #define UNREACHABLE() __builtin_unreachable()
    #else
        #define UNREACHABLE()
    #endif
#endif
