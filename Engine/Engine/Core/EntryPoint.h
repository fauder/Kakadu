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

	auto application = Engine::CreateApplication( flags );
	application->Run();
	delete application;
}
