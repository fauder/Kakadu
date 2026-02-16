#pragma once

// Engine Includes.
#include "Application.h"

extern Engine::Application* Engine::CreateApplication( Engine::BitFlags< Engine::CreationFlags > );

int main( int argc, char** argv )
{
	Engine::BitFlags< Engine::CreationFlags > flags;

	if( argc > 1 )
	{
		if( strcmp( argv[ 1 ], "DISABLE_IMGUI" ) == 0 )
			flags.Set( Engine::CreationFlags::OnStart_DisableImGui );
		else if( strcmp( argv[ 1 ], "DISABLE_GAMMA_CORRECTION" ) == 0 )
			flags.Set( Engine::CreationFlags::OnStart_DisableGammaCorrection );
	}

	std::cout << "====== KAKADU ENGINE INITIALIZATION LOGS ======\n\n";
	const auto begin = std::chrono::system_clock::now();
	auto application = Engine::CreateApplication( flags );
	const auto end = std::chrono::system_clock::now();
	std::cout << "\nTotal initialization: " << std::chrono::duration_cast< std::chrono::milliseconds >( ( end - begin ) ).count() << " ms.\n\n";
	std::cout << "===============================================\n\n";
	application->Run();
	delete application;
}
