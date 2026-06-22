#pragma once

// Engine Includes.
#include "Application.h"
#include "BitFlags.hpp"

// std Includes.
#include <cstring>
#include <iostream>

/* This macro is defined once per client project in a dedicated EntryPoint.cpp.
 *
 * Usage (EntryPoint.cpp):
 *   #include "MyApplication.h"
 *   #include "Engine/Core/EntryPoint.h"
 *   KAKADU_APP( MyApplication )
 */
#define KAKADU_APP( AppType )                                                   \
    int main( int argc, char** argv )                                           \
    {                                                                           \
        Kakadu::BitFlags< Kakadu::CreationFlags > flags;                        \
        if( argc > 1 && strcmp( argv[ 1 ], "DISABLE_IMGUI" ) == 0 )             \
            flags.Set( Kakadu::CreationFlags::OnStart_DisableImGui );           \
        std::cout << "====== KAKADU ENGINE INITIALIZATION LOGS ======\n\n";     \
        AppType app( flags );                                                   \
        app.Run();                                                              \
        std::cout << "\n===============================================\n\n";   \
    }
