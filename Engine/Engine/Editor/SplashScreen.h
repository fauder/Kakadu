#pragma once

// std Includes.
#include <string>

namespace Engine::Editor
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
		unsigned int tex_id;
		int source_width, source_height;
	};
}
