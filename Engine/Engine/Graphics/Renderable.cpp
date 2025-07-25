// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engine Includes.
#include "Renderable.h"

namespace Engine
{
	Renderable::Renderable()
		:
		transform( nullptr ),
		mesh( nullptr ),
		material( nullptr ),
		is_enabled( false ),
		is_receiving_shadows( false ),
		is_casting_shadows( false )
	{
	}

	Renderable::Renderable( const Mesh* mesh, Material* material, Transform* transform, const bool receive_shadows, const bool cast_shadows )
		:
		transform( transform ),
		mesh( mesh ),
		material( material ),
		is_enabled( true ),
		is_receiving_shadows( receive_shadows ),
		is_casting_shadows( cast_shadows )
	{
#if defined( _DEBUG ) || defined( _EDITOR )
		if( mesh->VertexCount() == 0 )
		{
		#if defined( _WIN32 ) && defined( _DEBUG )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "Renderable construction attempt via uninitialized Mesh!" );
		#endif // _WIN32 && _DEBUG

			ServiceLocator< GLLogger >::Get().Error( "Renderable construction attempt via uninitialized Mesh!" );
		}
#endif // _DEBUG || _EDITOR
	}

	Renderable::~Renderable()
	{
	}

	void Renderable::SetMesh( const Mesh* mesh )
	{
		this->mesh = mesh;
	}

	void Renderable::SetMaterial( Material* material )
	{
		this->material = material;
	}

	void Renderable::Enable()
	{
		is_enabled = true;
	}

	void Renderable::Disable()
	{
		is_enabled = false;
	}

	void Renderable::ToggleOnOff()
	{
		is_enabled = !is_enabled;
	}

	void Renderable::ToggleOnOff( const bool enable )
	{
		is_enabled = enable;
	}

	void Renderable::ToggleShadowReceiving( const bool enable )
	{
		is_receiving_shadows = enable;
	}

	void Renderable::ToggleShadowCasting( const bool enable )
	{
		is_casting_shadows = enable;
	}
}
