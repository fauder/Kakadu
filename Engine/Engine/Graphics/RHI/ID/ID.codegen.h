// No #pragma once: DEFINE_RHI_ID must be re-defined on each include so individual ID headers can #undef it after use.

// Engine Includes.
#include "Core/Macros_SpaceshipOperator.h"
#include "Core/Types.h"

#define DEFINE_RHI_ID_U8( type_name )                           \
struct type_name##ID                                            \
{                                                               \
    DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( type_name##ID );  \
    explicit operator bool() const { return id > 0u; }          \
    void Reset() { id = 0u; }                                   \
    Kakadu::u8 id = 0u;                                         \
};

#define DEFINE_RHI_ID_U16( type_name )                          \
struct type_name##ID                                            \
{                                                               \
    DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( type_name##ID );  \
    explicit operator bool() const { return id > 0u; }          \
    void Reset() { id = 0u; }                                   \
    Kakadu::u16 id = 0u;                                        \
};

#define DEFINE_RHI_ID_U32( type_name )                          \
struct type_name##ID                                            \
{                                                               \
    DEFAULT_EQUALITY_AND_SPACESHIP_OPERATORS( type_name##ID );  \
    explicit operator bool() const { return id > 0u; }          \
    void Reset() { id = 0u; }                                   \
    Kakadu::u32 id = 0u;                                        \
};

#define DEFINE_RHI_ID( type_name ) DEFINE_RHI_ID_U32( type_name )
