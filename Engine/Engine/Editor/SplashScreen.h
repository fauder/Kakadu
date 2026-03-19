#pragma once

// Engine Includes.
#include "Core/Types.h"

// std Includes.
#include <string>

namespace Kakadu::Editor
{
	class SplashScreen
	{
	public:
		SplashScreen( const std::string& image_file_name );
		~SplashScreen();

		void RenderOnce() const;
		float AspectRatio() const { return ( float )source_width / source_height; }

	private:
		void CreateTexture();

	private:
		u32 tex_id;
		int source_width, source_height;
	};
}
