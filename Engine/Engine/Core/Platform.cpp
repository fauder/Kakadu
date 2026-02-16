#ifdef _WIN32
// Windows Includes.
#include <ShlObj.h>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include "../Asset/Resource/Resource.h"
#endif // _WIN32

// Engine Includes.
#include "Graphics/Graphics.h"
#include "Optimization.h"
#include "Platform.h"
#include "Utility.hpp"
#ifdef _EDITOR
#include "Editor/SplashScreen.h"  
#include "Asset/Paths.h"
#endif // _EDITOR

// Vendor Includes.
#include "GLFW/glfw3.h"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_NATIVE_INCLUDE_NONE
#include "GLFW/glfw3native.h"  
#endif // _WIN32

#include <ImGui/backends/imgui_impl_glfw.h>
#include <Tracy/tracy/Tracy.hpp>

// std Includes.
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <thread>

namespace Platform
{
	struct WindowState
	{
		bool is_focused   = true;
		bool is_iconified = false;
	};

	internal_variable WindowState WINDOW_STATE;

	internal_variable GLFWwindow* SPLASH_WINDOW = nullptr;
	internal_variable GLFWwindow* MAIN_WINDOW   = nullptr;

	internal_variable bool KEYS_THAT_ARE_PRESSED[ ( int )KeyCode::KEY_LAST + 1 ] = { 0 };
	internal_variable bool KEYS_THAT_WERE_PRESSED[ ( int )KeyCode::KEY_LAST + 1 ] = { 0 };
	internal_variable float MOUSE_CURSOR_X_POS = 0.0f, MOUSE_CURSOR_Y_POS = 0.0f;
	internal_variable float MOUSE_CURSOR_X_DELTA = 0.0f, MOUSE_CURSOR_Y_DELTA = 0.0f;
	internal_variable float MOUSE_SCROLL_X_OFFSET = 0.0f, MOUSE_SCROLL_Y_OFFSET = 0.0f;
	internal_variable float MOUSE_SENSITIVITY = 0.004f;
	internal_variable bool MOUSE_CAPTURE_IS_RESET = true;
	internal_variable bool MOUSE_CAPTURE_ENABLED = false;

	internal_variable std::array< bool, GLFW_MOUSE_BUTTON_LAST > MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME{ false };
	internal_variable std::array< MouseButtonAction, GLFW_MOUSE_BUTTON_LAST > MOUSE_BUTTON_STATES{ MouseButtonAction::RELEASE };

	internal_variable std::function< void( const KeyCode key_code, const KeyAction action, const KeyMods mods )				> KEYBOARD_CALLBACK;
	internal_variable std::function< void( const MouseButton button, const MouseButtonAction action, const KeyMods mods )	> MOUSE_BUTTON_CALLBACK;
	internal_variable std::function< void( const float x_offset, const float y_offset )										> MOUSE_SCROLL_CALLBACK;
	internal_variable std::function< void( const int width_new_pixels, const int height_new_pixels )						> FRAMEBUFFER_RESIZE_CALLBACK;

#ifdef _EDITOR
	internal_variable std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) > GL_DEBUG_OUTPUT_CALLBACK;
#endif // _EDITOR

	internal_function void OnResizeWindow( GLFWwindow* window, const int width_new_pixels, const int height_new_pixels )
	{
		if( glfwGetMouseButton( MAIN_WINDOW, GLFW_MOUSE_BUTTON_LEFT ) != GLFW_RELEASE )
			return;
		
		glfwSetWindowSize( window, width_new_pixels, height_new_pixels );
		/* It is the client application's (renderer's) responsibility to call glViewport(). */

		if( FRAMEBUFFER_RESIZE_CALLBACK )
			FRAMEBUFFER_RESIZE_CALLBACK( width_new_pixels, height_new_pixels );
	}

	internal_function void OnKeyboardEvent( GLFWwindow* window, const int key_code, const int scan_code, const int action, const int mods )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return;

		if( KEYBOARD_CALLBACK )
			KEYBOARD_CALLBACK( KeyCode( key_code ), KeyAction( action ), KeyMods( mods ) );
	}

	internal_function void OnMouseCursorPositionChanged( GLFWwindow* window, const double x_position, const double y_position )
	{
		if( ImGui::GetIO().WantCaptureMouse )
			return;

		if( MOUSE_CAPTURE_IS_RESET )
		{
			MOUSE_CURSOR_X_POS = ( float )x_position;
			MOUSE_CURSOR_Y_POS = ( float )y_position;
			MOUSE_CAPTURE_IS_RESET = false;
		}

		MOUSE_CURSOR_X_DELTA = MOUSE_SENSITIVITY * ( ( float )x_position - MOUSE_CURSOR_X_POS );
		MOUSE_CURSOR_Y_DELTA = MOUSE_SENSITIVITY * ( ( float )y_position - MOUSE_CURSOR_Y_POS );

		MOUSE_CURSOR_X_POS = ( float )x_position;
		MOUSE_CURSOR_Y_POS = ( float )y_position;
	}

	internal_function void OnMouseScrolled( GLFWwindow* window, const double x_offset, const double y_offset )
	{
		if( ImGui::GetIO().WantCaptureMouse )
			return;

		MOUSE_SCROLL_X_OFFSET = ( float )x_offset;
		MOUSE_SCROLL_Y_OFFSET = ( float )y_offset;

		if( MOUSE_SCROLL_CALLBACK )
			MOUSE_SCROLL_CALLBACK( MOUSE_SCROLL_X_OFFSET, MOUSE_SCROLL_Y_OFFSET );
	}

	internal_function void OnMouseButtonEvent( GLFWwindow* window, const int button, const int action, const int mods )
	{
		if( ImGui::GetIO().WantCaptureMouse )
			return;

		if( MOUSE_BUTTON_CALLBACK )
			MOUSE_BUTTON_CALLBACK( MouseButton( button ), MouseButtonAction( action ), KeyMods( mods ) );
	}

#ifdef _EDITOR
	internal_function void OnDebugOutput( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters )
	{
		GL_DEBUG_OUTPUT_CALLBACK( source, type, id, severity, length, message, parameters );
	}
#endif // _EDITOR

	/* GLAD needs the created window's context made current BEFORE it is initialized. */
	internal_function void InitializeGLAD()
	{
		if( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
			throw std::runtime_error( "ERROR::GRAPHICS::GLAD::FAILED_TO_INITIALIZE!" );
	}

#ifdef _EDITOR
	internal_function void CreateGLDebugContext()
	{
		int gl_debug_context_flags = 0;
		glGetIntegerv( GL_CONTEXT_FLAGS, &gl_debug_context_flags );

		if( gl_debug_context_flags & GL_CONTEXT_FLAG_DEBUG_BIT )
			std::cout << "OpenGL debug context is created successfully.\n";
		else
			std::cerr << "Could not create OpenGL debug context!\n";
	}

	internal_function void RegisterGLDebugOutputCallback()
	{
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

		glDebugMessageCallback( OnDebugOutput, nullptr );
	}
#endif // _EDITOR

	internal_function void RegisterFrameBufferResizeCallback()
	{
		glfwSetFramebufferSizeCallback( MAIN_WINDOW, OnResizeWindow );
	}

	internal_function void RegisterMousePositionChangeCallback()
	{
		glfwSetCursorPosCallback( MAIN_WINDOW, OnMouseCursorPositionChanged );
	}

	internal_function void RegisterMouseScrollCallback()
	{
		glfwSetScrollCallback( MAIN_WINDOW, OnMouseScrolled );
	}

	internal_function void RegisterWindowFocusCallback( GLFWwindow* window, int focused )
	{
		auto* state = static_cast< WindowState* >( glfwGetWindowUserPointer( window ) );
		state->is_focused = ( focused == GLFW_TRUE );
	}

	internal_function void RegisterWindowIconifyCallback( GLFWwindow* window, int iconified )
	{
		auto* state = static_cast< WindowState* >( glfwGetWindowUserPointer( window ) );
		state->is_iconified = ( iconified == GLFW_TRUE );
	}

	struct WindowPositionAndSize
	{
		int x_pos, y_pos, width, height;
	};

	internal_function bool ReadLastKnownWindowSizeFromFile( WindowPositionAndSize& main_window_info, WindowPositionAndSize& console_window_info )
	{
		main_window_info.x_pos = main_window_info.y_pos = main_window_info.width = main_window_info.height = -1;

		if( auto config_file = std::ifstream( "window.cfg" ) )
		{
			std::string token_string;

			auto ParseToken = [ & ]( int& token_to_set, const char* token_name )
			{
				if( config_file >> token_string && token_string == token_name )
				{
					if( config_file >> token_string /* '=' */ >> token_string && not token_string.empty() )
					{
						token_to_set = std::stoi( token_string );
						return true;
					}
				}

				return false;
			};

			return 
				ParseToken( main_window_info.x_pos,  "main_x_pos" ) &&
				ParseToken( main_window_info.y_pos,  "main_y_pos" ) &&
				ParseToken( main_window_info.width,  "main_width" ) &&
				ParseToken( main_window_info.height, "main_height" ) &&
				ParseToken( console_window_info.x_pos,  "console_x_pos" ) &&
				ParseToken( console_window_info.y_pos,  "console_y_pos" ) &&
				ParseToken( console_window_info.width,  "console_width" ) &&
				ParseToken( console_window_info.height, "console_height" );
		}

		return false;
	}

	internal_function void CenterWindow( GLFWwindow* window, const int width_pixels, const int height_pixels )
	{
		const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );

		const auto max_width = mode->width;
		const auto max_height = mode->height;
		glfwSetWindowMonitor( window, NULL, ( max_width / 2 ) - ( width_pixels / 2 ), ( max_height / 2 ) - ( height_pixels / 2 ), width_pixels, height_pixels, GLFW_DONT_CARE );
	}

	/*
	 * Initialization:
	 */

	void InitializeAndCreateWindows( const bool enable_vsync )
	{
#ifdef _WIN32
		SetConsoleTitle( L"Kakadu Console" );
#endif

		glfwInit();

#ifdef _EDITOR
		glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, true );
#endif // _EDITOR
		glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
		glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 6 );
		glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

		//glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE ); // Needed for Mac OS X.

		/*
		 * 1) Create Splash Window
		 */

		glfwWindowHint( GLFW_DECORATED, GLFW_FALSE ); // No decorations during splash screen.
		glfwWindowHint( GLFW_VISIBLE,   GLFW_FALSE ); // Start hidden as we will resize it shortly.

		SPLASH_WINDOW = glfwCreateWindow( 100, 100, "Kakadu", nullptr, nullptr ); // Size doesn't matter; will be resized shortly.
		if( !SPLASH_WINDOW )
			throw std::runtime_error( "ERROR::PLATFORM::GLFW::FAILED TO CREATE SPLASH GLFW WINDOW!"  );

		glfwMakeContextCurrent( SPLASH_WINDOW );
		glfwSwapInterval( ( int )enable_vsync ); // Adaptive v-sync (interval >1) is not supported.

		// GLAD needs the created window's context made current BEFORE it is initialized.
		InitializeGLAD();

#ifdef _EDITOR
		Engine::Editor::SplashScreen splash_screen( ENGINE_TEXTURE_PATH_ABSOLUTE( "splash_screen.png" ) );

		int splash_width, splash_height;
		if( not GetMainMonitorResolution( splash_width, splash_height ) )
		{
			splash_width  = 800;
			splash_height = 600;
		}
		else
		{
			float splash_aspect_ratio = splash_screen.AspectRatio();

			splash_width /= 3;
			splash_height = ( int )( splash_width / splash_aspect_ratio );
		}

		glfwSetWindowSize( SPLASH_WINDOW, splash_width, splash_height );
		CenterWindow( SPLASH_WINDOW, splash_width, splash_height );
		glfwShowWindow( SPLASH_WINDOW );

		splash_screen.RenderOnce();
#endif
		
		/*
		 * 2) Create Main Window (Hidden, Decorated, Shared Context)
		 */

		glfwWindowHint( GLFW_DECORATED, GLFW_TRUE );
		glfwWindowHint( GLFW_VISIBLE,   GLFW_FALSE ); // Start hidden until init. finishes and we replace the splash window with this one.

		WindowPositionAndSize main_window_info, console_window_info;
		const bool known_size_is_used = ReadLastKnownWindowSizeFromFile( main_window_info, console_window_info );

		if( known_size_is_used )
		{
#ifdef _WIN32
			// Set console window to last known pos/size.
			SetWindowPos( GetConsoleWindow(), nullptr, console_window_info.x_pos, console_window_info.y_pos, console_window_info.width, console_window_info.height, SWP_NOZORDER );
#endif // _WIN32
		}
		else
		{
			// Revert:
			main_window_info.width  = 800;
			main_window_info.height = 600;
		}

		MAIN_WINDOW = glfwCreateWindow( main_window_info.width, main_window_info.height, "Kakadu", nullptr, SPLASH_WINDOW );
		if( MAIN_WINDOW == nullptr )
		{
			glfwTerminate();
			throw std::runtime_error( "ERROR::PLATFORM::GLFW::FAILED TO CREATE MAIN GLFW WINDOW!" );
		}

		// Switch current context to main:
		glfwMakeContextCurrent( MAIN_WINDOW );

#ifdef _EDITOR
		CreateGLDebugContext();
		RegisterGLDebugOutputCallback();
#endif // _EDITOR

		if( known_size_is_used )
			glfwSetWindowPos( MAIN_WINDOW, main_window_info.x_pos, main_window_info.y_pos );
		else
			CenterWindow( main_window_info.width, main_window_info.height );

		SetWindowIcon();

		RegisterFrameBufferResizeCallback();
		RegisterMousePositionChangeCallback();
		RegisterMouseScrollCallback();

		glfwSetWindowUserPointer( MAIN_WINDOW, &WINDOW_STATE );
		glfwSetWindowFocusCallback(	 MAIN_WINDOW, RegisterWindowFocusCallback );
		glfwSetWindowIconifyCallback( MAIN_WINDOW, RegisterWindowIconifyCallback );
	}

	void DestroySplashScreenAndSwitchToMainWindow()
	{
		if( SPLASH_WINDOW )
		{
			glfwDestroyWindow( SPLASH_WINDOW );
			SPLASH_WINDOW = nullptr;
		}

		glfwShowWindow( MAIN_WINDOW );
	}

	/*
	 * Window/Framebuffer:
	 */

	void ResizeWindow( const int width_new_pixels, const int height_new_pixels )
	{
		OnResizeWindow( MAIN_WINDOW, width_new_pixels, height_new_pixels );
	}

	void MinimizeWindow()
	{
		glfwIconifyWindow( MAIN_WINDOW );
	}

	void MaximizeWindow()
	{
		glfwMaximizeWindow( MAIN_WINDOW );
	}

	void RestoreWindow()
	{
		glfwRestoreWindow( MAIN_WINDOW );
	}

	void HideWindow()
	{
		glfwHideWindow( MAIN_WINDOW );
	}

	void ShowWindow()
	{
		glfwShowWindow( MAIN_WINDOW );
	}

	void SetFramebufferResizeCallback( std::function< void( const int width_new_pixels, const int height_new_pixels ) > callback )
	{
		FRAMEBUFFER_RESIZE_CALLBACK = callback;

		ImGui_ImplGlfw_RestoreCallbacks( MAIN_WINDOW );
		glfwSetFramebufferSizeCallback( MAIN_WINDOW, OnResizeWindow );
		ImGui_ImplGlfw_InstallCallbacks( MAIN_WINDOW );
	}

	bool IsWindowFocused()
	{
		return WINDOW_STATE.is_focused;
	}

	bool IsWindowIconified()
	{
		return WINDOW_STATE.is_iconified;
	}
	
	Engine::Vector2I GetFramebufferSizeInPixels()
	{
		int width, height;
		glfwGetFramebufferSize( MAIN_WINDOW, &width, &height );
		return Engine::Vector2I( width, height );
	}

	int GetFramebufferWidthInPixels()
	{
		return GetFramebufferSizeInPixels().X();
	}

	int GetFramebufferHeightInPixels()
	{
		return GetFramebufferSizeInPixels().Y();
	}

	float GetAspectRatio()
	{
		int width, height;
		glfwGetFramebufferSize( MAIN_WINDOW, &width, &height );
		return float( width ) / height;
	}

	bool GetMainMonitorResolution( int& width, int& height )
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if( !monitor )
		{
			std::cerr << "Could not get the primary monitor!\n";
			return false;
		}

		const GLFWvidmode* mode = glfwGetVideoMode( monitor );
		if( !mode )
		{
			std::cerr << "Could not get video mode of the primary monitor!\n";
			return false;
		}

		width  = mode->width;
		height = mode->height;

		return true;
	}

	std::uint16_t GetMainMonitorRefreshRate()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		if( monitor )
		{
			const GLFWvidmode* mode = glfwGetVideoMode( monitor );

			if( mode )
				return mode->refreshRate;
			else
				std::cerr << "Could not get video mode of the primary monitor!\n";
		}
		else
			std::cerr << "Could not get the primary monitor!\n";

		return -1;
	}

	void CenterWindow( const int width_pixels, const int height_pixels )
	{
		CenterWindow( MAIN_WINDOW, width_pixels, height_pixels );
	}

	bool SetWindowIcon()
	{
#ifdef _WIN32
		HINSTANCE instance_handle = GetModuleHandle( NULL ); // Get handle to the current module.
		HICON icon_handle = ( HICON )LoadImage(
			instance_handle,					// Use the current executable.
			MAKEINTRESOURCE( ENGINE_ICON_ID ),	// The ID from the resource.
			IMAGE_ICON,							// We're loading an icon.
			0,									// Width (default).
			0,									// Height (default).
			LR_DEFAULTSIZE						// Default size.
		);

		if( !icon_handle ) {
			DWORD error = GetLastError();
			std::cout << "LoadImage failed with error: " << error << std::endl;
		}

		if( icon_handle )
		{
			ICONINFO icon_info;
			if( GetIconInfo( icon_handle, &icon_info ) )
			{
				BITMAP bmp;
				if( GetObject( icon_info.hbmColor, sizeof( bmp ), &bmp ) )
				{
					// Allocate buffer for icon pixels (RGBA, assuming 32bpp):
					std::vector< unsigned char > pixels( bmp.bmWidth * bmp.bmHeight * 4 );

					// Prepare to extract pixel data:
					BITMAPINFO bmi              = {};
					bmi.bmiHeader.biSize        = sizeof( BITMAPINFOHEADER );
					bmi.bmiHeader.biWidth       = bmp.bmWidth;
					bmi.bmiHeader.biHeight      = -bmp.bmHeight; // Negative height for top-down rows.
					bmi.bmiHeader.biPlanes      = 1;
					bmi.bmiHeader.biBitCount    = 32; // Request 32bpp (RGBA)
					bmi.bmiHeader.biCompression = BI_RGB;

					// Get pixel data in 32bpp format:
					HDC hdc = GetDC( NULL );
					if( GetDIBits( hdc, icon_info.hbmColor, 0, bmp.bmHeight, pixels.data(), &bmi, DIB_RGB_COLORS ) )
					{
						// Ensure the pixel format is RGBA (convert BGRA to RGBA if necessary):
						for( size_t i = 0; i < pixels.size(); i += 4 )
							std::swap( pixels[ i ], pixels[ i + 2 ] ); // Swap R and B.

						GLFWimage icon[ 1 ];
						icon[ 0 ].width  = bmp.bmWidth;
						icon[ 0 ].height = bmp.bmHeight;
						icon[ 0 ].pixels = pixels.data();

						glfwSetWindowIcon( MAIN_WINDOW, 1, icon );

						ReleaseDC( NULL, hdc );

						return true;
					}

					ReleaseDC( NULL, hdc );
				}
			}
		}
		
		return false;
#else
		throw std::logic_error( "Setting the window icon is not implemented for OSes other than Windows yet!" );
#endif
	}

	void SwapBuffers()
	{
		glfwSwapBuffers( MAIN_WINDOW );
	}

	/*
	 * Events:
	 */

	void PollEvents()
	{
		MOUSE_CURSOR_X_DELTA = MOUSE_CURSOR_Y_DELTA = 0.0f;
		MOUSE_SCROLL_X_OFFSET = MOUSE_SCROLL_Y_OFFSET = 0.0f;

		for( auto button_index = 0; button_index < GLFW_MOUSE_BUTTON_LAST; button_index++ )
		{
			const auto action_this_frame = MouseButtonAction( glfwGetMouseButton( MAIN_WINDOW, button_index ) );
			MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME[ button_index ] = action_this_frame != MOUSE_BUTTON_STATES[ button_index ]; // First update status change.
			MOUSE_BUTTON_STATES[ button_index ] = action_this_frame; // Then update actual button state.
		}

		std::copy_n( KEYS_THAT_ARE_PRESSED, sizeof( KEYS_THAT_ARE_PRESSED ), KEYS_THAT_WERE_PRESSED );

		for( auto i = 0; i < ( int )KeyCode::KEY_LAST + 1; i++ )
			KEYS_THAT_ARE_PRESSED[ i ] = glfwGetKey( MAIN_WINDOW, i ) == GLFW_PRESS;

		glfwPollEvents();
	}

#ifdef _EDITOR
	/*
	 * GL Debug Output:
	 */

	void SetGLDebugOutputCallback( std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) > callback )
	{
		GL_DEBUG_OUTPUT_CALLBACK = callback;
	}
#endif // _EDITOR

	/*
	 * Keyboard IO:
	 */

	void SetKeyboardEventCallback( std::function< void( const KeyCode key_code, const KeyAction action, const KeyMods mods ) > callback )
	{
		KEYBOARD_CALLBACK = callback;

		ImGui_ImplGlfw_RestoreCallbacks( MAIN_WINDOW );
		glfwSetKeyCallback( MAIN_WINDOW, OnKeyboardEvent );
		ImGui_ImplGlfw_InstallCallbacks( MAIN_WINDOW );
	}

	bool IsKeyPressed( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return KEYS_THAT_ARE_PRESSED[ ( int )key_code ];
	}

	bool IsKeyPressedThisFrame( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return KEYS_THAT_ARE_PRESSED[ ( int )key_code ] && not KEYS_THAT_WERE_PRESSED[ ( int )key_code ];
	}

	bool IsKeyReleased( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return !KEYS_THAT_ARE_PRESSED[ ( int )key_code ];
	}

	bool IsKeyReleasedThisFrame( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return KEYS_THAT_WERE_PRESSED[ ( int )key_code ] && not KEYS_THAT_ARE_PRESSED[ ( int )key_code ];
	}

	bool IsKeyModifierHeldDown( const KeyMods key_mod )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		switch( key_mod )
		{
			case KeyMods::SHIFT:		return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_LEFT_SHIFT	) ) || glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_RIGHT_SHIFT	) );
			case KeyMods::CONTROL:		return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_LEFT_CONTROL	) ) || glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_RIGHT_CONTROL	) );
			case KeyMods::ALT:			return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_LEFT_ALT		) ) || glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_RIGHT_ALT		) );
			case KeyMods::SUPER:		return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_LEFT_SUPER	) ) || glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_RIGHT_SUPER	) );
			case KeyMods::CAPS_LOCK:	return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_CAPS_LOCK		) );
			case KeyMods::NUM_LOCK:		return glfwGetKey( MAIN_WINDOW, int( KeyCode::KEY_NUM_LOCK		) );

			default:
				UNREACHABLE();
				return false;
		}
	}

	/*
	 * Mouse IO:
	 */

	void SetMouseButtonEventCallback( std::function< void( const MouseButton button, const MouseButtonAction action, const KeyMods mods ) > callback )
	{
		MOUSE_BUTTON_CALLBACK = callback;

		ImGui_ImplGlfw_RestoreCallbacks( MAIN_WINDOW );
		glfwSetMouseButtonCallback( MAIN_WINDOW, OnMouseButtonEvent );
		ImGui_ImplGlfw_InstallCallbacks( MAIN_WINDOW );
	}

	void SetMouseScrollEventCallback( std::function< void( const float x_offset, const float y_offset ) > callback )
	{
		MOUSE_SCROLL_CALLBACK = callback;

		ImGui_ImplGlfw_RestoreCallbacks( MAIN_WINDOW );
		glfwSetScrollCallback( MAIN_WINDOW, OnMouseScrolled );
		ImGui_ImplGlfw_InstallCallbacks( MAIN_WINDOW );
	}

	bool IsMouseButtonPressed( const MouseButton mouse_button )
	{
		return glfwGetMouseButton( MAIN_WINDOW, ( int )mouse_button ) == GLFW_PRESS;
	}

	bool IsMouseButtonPressed_ThisFrame( const MouseButton mouse_button )
	{
		return MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME[ ( int )mouse_button ] && IsMouseButtonPressed( mouse_button );
	}
	
	bool IsMouseButtonReleased( const MouseButton mouse_button )
	{
		return glfwGetMouseButton( MAIN_WINDOW, ( int )mouse_button ) == GLFW_RELEASE;
	}

	bool IsMouseButtonReleased_ThisFrame( const MouseButton mouse_button )
	{
		return MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME[ ( int )mouse_button ] && IsMouseButtonReleased( mouse_button );
	}

	void ResetMouseDeltas()
	{
		MOUSE_CAPTURE_IS_RESET = true;
		MOUSE_CURSOR_X_DELTA = MOUSE_CURSOR_Y_DELTA = 0.0f;
	}

	void CaptureMouse( const bool should_capture )
	{
		if( !MOUSE_CAPTURE_ENABLED && should_capture )
			glfwSetInputMode( MAIN_WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		else if( MOUSE_CAPTURE_ENABLED && !should_capture )
		{
			MOUSE_CAPTURE_IS_RESET = true;
			glfwSetInputMode( MAIN_WINDOW, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
		}

		MOUSE_CAPTURE_ENABLED = should_capture;
	}

	float GetMouseSensitivity()
	{
		return MOUSE_SENSITIVITY;
	}

	void SetMouseSensitivity( const float new_sensitivity )
	{
		MOUSE_SENSITIVITY = new_sensitivity;
	}

	std::pair< float, float > GetMouseCursorDeltas()
	{
		return { MOUSE_CURSOR_X_DELTA, MOUSE_CURSOR_Y_DELTA };
	}

	void SetMouseCursorPosition( const float x_position, const float y_position )
	{
		glfwSetCursorPos( MAIN_WINDOW, x_position, y_position );
	}

	void OffsetMouseCursorPosition( const float delta_x, const float delta_y )
	{
		SetMouseCursorPosition( MOUSE_CURSOR_X_POS + delta_x, MOUSE_CURSOR_Y_POS + delta_y );
	}

	std::pair< float, float > GetMouseCursorPositions()
	{
		return { MOUSE_CURSOR_X_POS, MOUSE_CURSOR_Y_POS };
	}

	std::pair< float, float > GetMouseScrollOffsets()
	{
		return { MOUSE_SCROLL_X_OFFSET, MOUSE_SCROLL_Y_OFFSET };
	}

	/*
	 * File IO:
	 */

	std::optional< std::string > BrowseFileName( const std::vector< std::string >& filters, const std::string& prompt )
	{
#ifdef _WIN32
		OPENFILENAMEA dialog;
		char file_name_buffer[ 260 ];

		/* Initialize OPENFILENAMEA: */
		ZeroMemory( &dialog, sizeof( dialog ) );
		dialog.lStructSize = sizeof( dialog );
		dialog.hwndOwner = NULL;
		dialog.lpstrFile = file_name_buffer;
		if( !prompt.empty() )
			dialog.lpstrTitle = prompt.c_str();

		/* Convert vector of strings to vector of chars, with null characters inserted between the strings.
			we need a contiguous array of chars to present to lpstrFilter. */
		std::vector< char > filters_char_array_contiguous;
		for( auto& filter_string : filters )
		{
			filters_char_array_contiguous.insert( filters_char_array_contiguous.begin() + filters_char_array_contiguous.size(),
												  filter_string.cbegin(), filter_string.cend() );
			filters_char_array_contiguous.push_back( '\0' );
		}

		filters_char_array_contiguous.push_back( '\0' ); // This null character is used to _actually_ mark the end of the sequence.

		/* Set lpstrFile[ 0 ] to '\0' so that GetOpenFileNameA does not use the contents of file_name_buffer to initialize itself: */
		dialog.lpstrFile[ 0 ] = '\0';
		dialog.nMaxFile = sizeof( file_name_buffer );

		dialog.lpstrFilter     = filters_char_array_contiguous.data();
		dialog.nFilterIndex    = 1;
		dialog.lpstrFileTitle  = NULL;
		dialog.nMaxFileTitle   = 0;
		dialog.lpstrInitialDir = NULL;
		dialog.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST |
								 OFN_NOCHANGEDIR /* this is crucial, otherwise working dir. changes. We use non-windows APIs for that. */;

		/* Display the Open dialog box: */
		if( GetOpenFileNameA( &dialog ) == TRUE )
			return std::optional< std::string >( dialog.lpstrFile );

		return std::nullopt;
#endif // _WIN32

		throw std::logic_error( "Platform::BrowseFileName() not implemented for current platform." );
	}

	std::optional< std::string > BrowseDirectory( const std::string& title, const std::string& directory_path )
	{
#ifdef _WIN32
		local_persist auto co_initialized = SUCCEEDED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ) );

		IFileOpenDialog* file_open_dialog;

		// Create the FileOpenDialog object.
		if( SUCCEEDED( CoCreateInstance( CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast< void** >( &file_open_dialog ) ) ) )
		{
			file_open_dialog->SetTitle( Engine::Utility::String::ToWideString( title ).c_str() );

			/* Set default directory. */
			const std::wstring directory_path_wide( directory_path.begin(), directory_path.end() );
			if( !directory_path_wide.empty() )
			{
				IShellItem* default_directory_item;
				if( SUCCEEDED( SHCreateItemFromParsingName( directory_path_wide.c_str(), NULL, IID_PPV_ARGS( &default_directory_item ) ) ) )
					file_open_dialog->SetFolder( default_directory_item );
				default_directory_item->Release();
			}

			file_open_dialog->SetOptions( FOS_PICKFOLDERS );

			// Show the Open dialog box.
			if( SUCCEEDED( file_open_dialog->Show( NULL ) ) )
			{
				// Get the file name from the dialog box.
				IShellItem* pItem;
				if( SUCCEEDED( file_open_dialog->GetResult( &pItem ) ) )
				{
					PWSTR pszFilePath;

					// Display the file name to the user.
					if( SUCCEEDED( pItem->GetDisplayName( SIGDN_FILESYSPATH, &pszFilePath ) ) )
					{
						std::wstring selected_directory_path_wide( pszFilePath );
						CoTaskMemFree( pszFilePath );
						return Engine::Utility::String::ToNarrowString( selected_directory_path_wide );
					}
					pItem->Release();
				}
			}

			file_open_dialog->Release();
		}

		return std::nullopt;
#endif // _WIN32

		throw std::logic_error( "Platform::BrowseDirectory() not implemented for current platform." );
	}

	/*
	 * Time-keeping Facilities:
	 */

	float CurrentTime()
	{
		return static_cast< float >( glfwGetTime() );
	}

	/*
	 * Shutdown:
	 */

	void Shutdown()
	{
		/* Save window pos. and size to file: */
		{
			/* Main Window: */
			std::ofstream output_file( "window.cfg" );

			{
				int x_pos, y_pos;
				glfwGetWindowPos( MAIN_WINDOW, &x_pos, &y_pos );
				const auto framebuffer_size = GetFramebufferSizeInPixels();
				output_file << "main_x_pos = " << x_pos << "\n";
				output_file << "main_y_pos = " << y_pos << "\n";
				output_file << "main_width = " << framebuffer_size.X() << "\n";
				output_file << "main_height = " << framebuffer_size.Y() << "\n";
			}

			/* Console Window: */
			{
#ifdef _WIN32
				HWND hwnd = GetConsoleWindow();

				RECT rect{};
				GetWindowRect( hwnd, &rect );

				int x_pos  = rect.left;
				int y_pos  = rect.top;
				int width  = rect.right - rect.left;
				int height = rect.bottom - rect.top;

				output_file << "console_x_pos = " << x_pos << "\n";
				output_file << "console_y_pos = " << y_pos << "\n";
				output_file << "console_width = " << width << "\n";
				output_file << "console_height = " << height << "\n";
#endif // _WIN32
			}
		}

		glfwTerminate();
	}

	/*
	 * Other:
	 */

	void SetShouldClose( const bool value )
	{
		glfwSetWindowShouldClose( MAIN_WINDOW, value );
	}

	bool ShouldClose()
	{
		ZoneScoped;
		return glfwWindowShouldClose( MAIN_WINDOW );
	}

	void ChangeTitle( const char* new_title )
	{
		glfwSetWindowTitle( MAIN_WINDOW, new_title );
	}

	void* GetWindowHandle()
	{
		return reinterpret_cast< void* >( MAIN_WINDOW );
	}

	void LaunchWithDefaultProgram( const std::string& file_path )
	{
#ifdef _WIN32
		ShellExecute( nullptr, L"open",
					  Engine::Utility::String::ToWideString( file_path ).c_str(),
					  nullptr,
					  nullptr,
					  SW_SHOWNORMAL );
#else
		throw std::logic_error( "LaunchWithDefaultProgram() is not implemented for OSes other than Windows yet!" );
#endif // _WIN32
	}

	void Sleep( std::chrono::nanoseconds duration )
	{
		std::this_thread::sleep_for( duration );
	}

	/* Windows Only. */
#ifdef _WIN32
	void MoveConsoleWindowToNonPrimaryMonitor()
	{
		HWND console_hwnd = GetConsoleWindow();
		if( console_hwnd )
		{
			HMONITOR target_monitor = nullptr;

			EnumDisplayMonitors( nullptr,
								 nullptr,
								 []( HMONITOR monitor, HDC, LPRECT, LPARAM user_data ) -> BOOL
									{
										MONITORINFO info{};
										info.cbSize = sizeof( info );

										if( GetMonitorInfo( monitor, &info ) )
										{
											// Pick the non-primary monitor
											if( !( info.dwFlags & MONITORINFOF_PRIMARY ) )
											{
												*reinterpret_cast< HMONITOR* >( user_data ) = monitor;
												return FALSE; // stop enumeration
											}
										}

										return TRUE;
									},
								 reinterpret_cast< LPARAM >( &target_monitor ) );

			if( target_monitor )
			{
				MONITORINFO info{};
				info.cbSize = sizeof( info );
				GetMonitorInfo( target_monitor, &info );

				RECT work = info.rcWork;

				SetWindowPos( console_hwnd,
							  HWND_TOP,
							  work.left + 50,
							  work.top + 50,
							  800,
							  600,
							  SWP_SHOWWINDOW );
			}
		}
	}
#endif
}