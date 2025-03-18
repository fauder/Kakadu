#ifdef _WIN32
// Windows Includes.
#include <ShlObj.h>
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <Windows.h>
#include "../Asset/Resource/Resource.h"
#endif // _WIN32

// Engine Includes.
#include "Graphics/Graphics.h"
#include "Platform.h"
#include "Utility.hpp"

// Vendor Includes.
#include "GLFW/glfw3.h"
#include <ImGui/backends/imgui_impl_glfw.h>
#include <Tracy/tracy/Tracy.hpp>

// std Includes.
#include <iostream>
#include <stdexcept>

namespace Platform
{
	GLFWwindow* WINDOW = nullptr; // No need to expose this outside.
	float MOUSE_CURSOR_X_POS = 0.0f, MOUSE_CURSOR_Y_POS = 0.0f;
	float MOUSE_CURSOR_X_DELTA = 0.0f, MOUSE_CURSOR_Y_DELTA = 0.0f;
	float MOUSE_SCROLL_X_OFFSET = 0.0f, MOUSE_SCROLL_Y_OFFSET = 0.0f;
	float MOUSE_SENSITIVITY = 0.004f;
	bool MOUSE_CAPTURE_IS_RESET = true;
	bool MOUSE_CAPTURE_ENABLED = false;
	std::function< void( const KeyCode key_code, const KeyAction action, const KeyMods mods )	> KEYBOARD_CALLBACK;
	std::function< void( const int width_new_pixels, const int height_new_pixels )				> FRAMEBUFFER_RESIZE_CALLBACK;
#ifdef _EDITOR
	std::function< void( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters ) > GL_DEBUG_OUTPUT_CALLBACK;
#endif // _EDITOR

	void OnResizeWindow( GLFWwindow* window, const int width_new_pixels, const int height_new_pixels )
	{
		if( glfwGetMouseButton( WINDOW, GLFW_MOUSE_BUTTON_LEFT ) != GLFW_RELEASE )
			return;

		glfwSetWindowSize( window, width_new_pixels, height_new_pixels );

		/* It is the client application's (renderer's) responsibility to call glViewport(). */

		if( FRAMEBUFFER_RESIZE_CALLBACK )
			FRAMEBUFFER_RESIZE_CALLBACK( width_new_pixels, height_new_pixels );
	}

	void OnMouseCursorPositionChanged( GLFWwindow* window, const double x_position, const double y_position )
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

	void OnMouseScrolled( GLFWwindow* window, const double x_offset, const double y_offset )
	{
		if( ImGui::GetIO().WantCaptureMouse )
			return;

		MOUSE_SCROLL_X_OFFSET = ( float )x_offset;
		MOUSE_SCROLL_Y_OFFSET = ( float )y_offset;
	}

	void OnKeyboardEvent( GLFWwindow* window, const int key_code, const int scan_code, const int action, const int mods )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return;

		if( KEYBOARD_CALLBACK )
			KEYBOARD_CALLBACK( KeyCode( key_code ), KeyAction( action ), KeyMods( mods ) );
	}

#ifdef _EDITOR
	void OnDebugOutput( GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* parameters )
	{
		GL_DEBUG_OUTPUT_CALLBACK( source, type, id, severity, length, message, parameters );
	}
#endif // _EDITOR

	/* GLAD needs the created window's context made current BEFORE it is initialized. */
	void InitializeGLAD()
	{
		if( !gladLoadGLLoader( ( GLADloadproc )glfwGetProcAddress ) )
			throw std::logic_error( "ERROR::GRAPHICS::GLAD::FAILED_TO_INITIALIZE!" );
	}

#ifdef _EDITOR
	void RegisterGLDebugOutputCallback()
	{
		glEnable( GL_DEBUG_OUTPUT );
		glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS );

		glDebugMessageCallback( OnDebugOutput, nullptr );
	}
#endif // _EDITOR

	void RegisterFrameBufferResizeCallback()
	{
		glfwSetFramebufferSizeCallback( WINDOW, OnResizeWindow );
	}

	void RegisterMousePositionChangeCallback()
	{
		glfwSetCursorPosCallback( WINDOW, OnMouseCursorPositionChanged );
	}

	void RegisterMouseScrollCallback()
	{
		glfwSetScrollCallback( WINDOW, OnMouseScrolled );
	}

	/*
	 * Initialization:
	 */

	void InitializeAndCreateWindow( const int width_pixels, const int height_pixels, 
									const std::optional< int > msaa_sample_count,
									const bool enable_vsync )
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

		glfwWindowHint( GLFW_VISIBLE, GLFW_FALSE ); // Start hidden as we will move it shortly.
		if( msaa_sample_count.has_value() )
			glfwWindowHint( GLFW_SAMPLES, *msaa_sample_count );

		WINDOW = glfwCreateWindow( width_pixels, height_pixels, "Kakadu Application", nullptr, nullptr );
		if( WINDOW == nullptr )
		{
			glfwTerminate();
			throw std::logic_error( "ERROR::PLATFORM::GLFW::FAILED TO CREATE GLFW WINDOW!" );
		}

		CenterWindow( width_pixels, height_pixels );

		glfwShowWindow( WINDOW );

		glfwMakeContextCurrent( WINDOW );

		glfwSwapInterval( ( int )enable_vsync ); // Adaptive v-sync (interval >1) is not supported.

		// GLAD needs the created window's context made current BEFORE it is initialized.
		InitializeGLAD();

		if( msaa_sample_count.has_value() )
			glEnable( GL_MULTISAMPLE );

#ifdef _EDITOR
		int gl_debug_context_flags = 0;
		glGetIntegerv( GL_CONTEXT_FLAGS, &gl_debug_context_flags );

		if( gl_debug_context_flags & GL_CONTEXT_FLAG_DEBUG_BIT )
			std::cout << "OpenGL debug context is created successfully.\n";
		else
			std::cerr << "Could not create OpenGL debug context!\n";

		RegisterGLDebugOutputCallback();
#endif // _EDITOR

		SetWindowIcon();

		ResizeWindow( width_pixels, height_pixels );

		RegisterFrameBufferResizeCallback();
		RegisterMousePositionChangeCallback();
		RegisterMouseScrollCallback();
	}

	/*
	 * Window/Framebuffer:
	 */

	void ResizeWindow( const int width_new_pixels, const int height_new_pixels )
	{
		OnResizeWindow( WINDOW, width_new_pixels, height_new_pixels );
	}

	void MinimizeWindow()
	{
		glfwIconifyWindow( WINDOW );
	}

	void MaximizeWindow()
	{
		glfwMaximizeWindow( WINDOW );
	}

	void RestoreWindow()
	{
		glfwRestoreWindow( WINDOW );
	}

	void SetFramebufferResizeCallback( std::function< void( const int width_new_pixels, const int height_new_pixels ) > callback )
	{
		FRAMEBUFFER_RESIZE_CALLBACK = callback;

		ImGui_ImplGlfw_RestoreCallbacks( WINDOW );
		glfwSetFramebufferSizeCallback( WINDOW, OnResizeWindow );
		ImGui_ImplGlfw_InstallCallbacks( WINDOW );
	}

	Engine::Vector2I GetFramebufferSizeInPixels()
	{
		int width, height;
		glfwGetFramebufferSize( WINDOW, &width, &height );
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
		glfwGetFramebufferSize( WINDOW, &width, &height );
		return float( width ) / height;
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
		const GLFWvidmode* mode = glfwGetVideoMode( glfwGetPrimaryMonitor() );

		const auto max_width = mode->width;
		const auto max_height = mode->height;
		glfwSetWindowMonitor( WINDOW, NULL, ( max_width / 2 ) - ( width_pixels / 2 ), ( max_height / 2 ) - ( height_pixels / 2 ), width_pixels, height_pixels, GLFW_DONT_CARE );
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

						glfwSetWindowIcon( WINDOW, 1, icon );

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
		glfwSwapBuffers( WINDOW );
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
			const auto action_this_frame = MouseButtonAction( glfwGetMouseButton( WINDOW, button_index ) );
			MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME[ button_index ] = action_this_frame != MOUSE_BUTTON_STATES[ button_index ]; // First update status change.
			MOUSE_BUTTON_STATES[ button_index ] = action_this_frame; // Then update actual button state.
		}

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

		ImGui_ImplGlfw_RestoreCallbacks( WINDOW );
		glfwSetKeyCallback( WINDOW, OnKeyboardEvent );
		ImGui_ImplGlfw_InstallCallbacks( WINDOW );
	}

	bool IsKeyPressed( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return glfwGetKey( WINDOW, int( key_code ) ) == GLFW_PRESS;
	}

	bool IsKeyReleased( const KeyCode key_code )
	{
		if( ImGui::GetIO().WantCaptureKeyboard )
			return false;

		return glfwGetKey( WINDOW, int( key_code ) ) == GLFW_RELEASE;
	}

	/*
	 * Mouse IO:
	 */

	bool IsMouseButtonPressed( const MouseButton mouse_button )
	{
		return glfwGetMouseButton( WINDOW, ( int )mouse_button ) == GLFW_PRESS;
	}

	bool IsMouseButtonPressed_ThisFrame( const MouseButton mouse_button )
	{
		return MOUSE_BUTTON_STATUS_CHANGES_THIS_FRAME[ ( int )mouse_button ] && IsMouseButtonPressed( mouse_button );
	}
	
	bool IsMouseButtonReleased( const MouseButton mouse_button )
	{
		return glfwGetMouseButton( WINDOW, ( int )mouse_button ) == GLFW_RELEASE;
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
			glfwSetInputMode( WINDOW, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
		else if( MOUSE_CAPTURE_ENABLED && !should_capture )
		{
			MOUSE_CAPTURE_IS_RESET = true;
			glfwSetInputMode( WINDOW, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
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
		static auto co_initialized = SUCCEEDED( CoInitializeEx( NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE ) );

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

	void CleanUp()
	{
		glfwTerminate();
	}

	/*
	 * Other:
	 */

	void SetShouldClose( const bool value )
	{
		glfwSetWindowShouldClose( WINDOW, value );
	}

	bool ShouldClose()
	{
		ZoneScoped;
		return glfwWindowShouldClose( WINDOW );
	}

	void ChangeTitle( const char* new_title )
	{
		glfwSetWindowTitle( WINDOW, new_title );
	}

	void* GetWindowHandle()
	{
		return reinterpret_cast< void* >( WINDOW );
	}
}