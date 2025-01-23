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
		is_receiving_shadows( false )
	{
	}

	Renderable::Renderable( const Mesh* mesh, Material* material, Transform* transform, const bool receive_shadows )
		:
		transform( transform ),
		mesh( mesh ),
		material( material ),
		is_enabled( true ),
		is_receiving_shadows( receive_shadows )
	{
#ifdef _DEBUG
		if( mesh->VertexCount() == 0 )
		{
		#if defined( _WIN32 ) && defined( _DEBUG )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "Renderable construction attempt via uninitialized Mesh!" );
		#endif // _WIN32 && _DEBUG

			ServiceLocator< GLLogger >::Get().Error( "Renderable construction attempt via uninitialized Mesh!" );
		}
#endif // _DEBUG
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

	void Renderable::ToggleOn()
	{
		is_enabled = true;
	}

	void Renderable::ToggleOff()
	{
		is_enabled = false;
	}

	void Renderable::ToggleOnOrOff()
	{
		is_enabled = !is_enabled;
	}

	void Renderable::ToggleOnOrOff( const bool enable )
	{
		is_enabled = enable;
	}
}
