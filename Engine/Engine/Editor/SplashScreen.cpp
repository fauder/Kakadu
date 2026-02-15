// Engine Includes.
#include "SplashScreen.h"
#include "Graphics/Graphics.h"

// Vendor/stb Includes.
#include "stb/stb_image.h"
#include "GLFW/glfw3.h"

// std Includes.
#include <stdexcept>

namespace Engine::Editor
{
	SplashScreen::SplashScreen( const std::string& image_file_path )
		:
		tex_id( 0 ),
		source_width( 0 ),
		source_height( 0 )
	{
		// OpenGL expects uv coordinate v = 0 to be on the most bottom whereas stb loads image data with v = 0 to be top.
		stbi_set_flip_vertically_on_load( true );

		constexpr int DESIRED_CHANNELS = 4;

		int number_of_channels = 0;

		stbi_uc* pixels = stbi_load( image_file_path.c_str(),
									 &source_width,
									 &source_height,
									 &number_of_channels,
									 DESIRED_CHANNELS );

		if( !pixels )
			throw std::runtime_error( "Failed to load splash image!" );

		glGenTextures( 1, &tex_id );
		glBindTexture( GL_TEXTURE_2D, tex_id );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RGBA8,
					  source_width,
					  source_height,
					  0,
					  GL_RGBA,
					  GL_UNSIGNED_BYTE,
					  pixels );

		stbi_image_free( pixels );
	}

	SplashScreen::~SplashScreen()
	{
		if( tex_id )
			glDeleteTextures( 1, &tex_id );
	}

	void SplashScreen::RenderOnce() const
	{
		static GLuint program = 0;
		static GLuint vao = 0;

		if( program == 0 )
		{
			const char* vs_source = R"(
#version 460 core

const vec2 verts[3] = vec2[](
	vec2( -1.0, -1.0 ),
	vec2(  3.0, -1.0 ),
	vec2( -1.0,  3.0 )
);

out vec2 uv;

void main()
{
	gl_Position = vec4( verts[ gl_VertexID ], 0.0, 1.0 );
	uv = gl_Position.xy * 0.5 + 0.5;
}
)";

			const char* fs_source = R"(
#version 460 core

in vec2 uv;
out vec4 out_color;

uniform sampler2D splash;

void main()
{
	out_color = texture( splash, uv );
}
)";

			auto compile_shader = []( GLenum type, const char* src ) -> GLuint
			{
				GLuint shader = glCreateShader( type );
				glShaderSource( shader, 1, &src, nullptr );
				glCompileShader( shader );

				GLint ok = 0;
				glGetShaderiv( shader, GL_COMPILE_STATUS, &ok );
				if( !ok )
				{
					char log[ 1024 ];
					glGetShaderInfoLog( shader, sizeof( log ), nullptr, log );
					throw std::runtime_error( log );
				}

				return shader;
			};

			GLuint vs = compile_shader( GL_VERTEX_SHADER, vs_source );
			GLuint fs = compile_shader( GL_FRAGMENT_SHADER, fs_source );

			program = glCreateProgram();
			glAttachShader( program, vs );
			glAttachShader( program, fs );
			glLinkProgram( program );

			glDeleteShader( vs );
			glDeleteShader( fs );

			GLint linked = 0;
			glGetProgramiv( program, GL_LINK_STATUS, &linked );
			if( !linked )
			{
				char log[ 1024 ];
				glGetProgramInfoLog( program, sizeof( log ), nullptr, log );
				throw std::runtime_error( log );
			}

			glGenVertexArrays( 1, &vao );
		}

		glBindFramebuffer( GL_FRAMEBUFFER, 0 );

		int fb_width = 0;
		int fb_height = 0;
		glfwGetFramebufferSize( glfwGetCurrentContext(), &fb_width, &fb_height );
		glViewport( 0, 0, fb_width, fb_height );

		glDisable( GL_DEPTH_TEST );
		glDisable( GL_BLEND );
		glDisable( GL_CULL_FACE );

		glUseProgram( program );
		glBindVertexArray( vao );

		glActiveTexture( GL_TEXTURE0 );
		glBindTexture( GL_TEXTURE_2D, tex_id );
		glUniform1i( glGetUniformLocation( program, "splash" ), 0 );

		glDrawArrays( GL_TRIANGLES, 0, 3 );

		glfwSwapBuffers( glfwGetCurrentContext() );
		glFinish();
	}
}