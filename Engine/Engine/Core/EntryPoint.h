#pragma once

// Engine Includes.
#include "Application.h"

extern Kakadu::Application* Kakadu::CreateApplication( Kakadu::BitFlags< Kakadu::CreationFlags > );

int main( int argc, char** argv )
{
	Kakadu::BitFlags< Kakadu::CreationFlags > flags;

	if( argc > 1 )
	{
		if( strcmp( argv[ 1 ], "DISABLE_IMGUI" ) == 0 )
			flags.Set( Kakadu::CreationFlags::OnStart_DisableImGui );
		else if( strcmp( argv[ 1 ], "DISABLE_GAMMA_CORRECTION" ) == 0 )
			flags.Set( Kakadu::CreationFlags::OnStart_DisableGammaCorrection );
	}

	std::cout << "====== KAKADU ENGINE INITIALIZATION LOGS ======\n\n";
	const auto begin = std::chrono::system_clock::now();
	auto application = Kakadu::CreateApplication( flags );
	const auto end = std::chrono::system_clock::now();
	std::cout << "\nTotal initialization: " << std::chrono::duration_cast< std::chrono::milliseconds >( ( end - begin ) ).count() << " ms.\n\n";
	std::cout << "===============================================\n\n";
	application->Run();
	delete application;
}
