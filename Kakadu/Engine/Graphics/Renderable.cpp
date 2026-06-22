// Platform-specific Debug API includes.
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers
#include <windows.h> // For Visual Studio's OutputDebugString().
#endif // _WIN32

// Engine Includes.
#include "Renderable.h"

namespace Kakadu
{
	Renderable::Renderable()
		:
		is_enabled( false ),
		is_receiving_shadows( false ),
		is_casting_shadows( false ),
		transform( nullptr ),
		world_matrix( nullptr ),
		mesh( nullptr ),
		material( nullptr )
	{
	}

	Renderable::Renderable( const Mesh* mesh, Material* material, Transform* transform, const bool receive_shadows, const bool cast_shadows, const Matrix4x4* world_matrix )
		:
		is_enabled( true ),
		is_receiving_shadows( receive_shadows ),
		is_casting_shadows( cast_shadows ),
		transform( transform ),
		world_matrix( world_matrix ),
		mesh( mesh ),
		material( material )
	{
#if defined( _DEBUG ) || defined( _EDITOR )
		if( mesh->VertexCount() == 0 )
		{
		#if defined( _WIN32 ) && defined( _DEBUG )
			if( IsDebuggerPresent() )
				OutputDebugStringA( "Renderable construction attempt via uninitialized Mesh!" );
		#endif // _WIN32 && _DEBUG

			Log::Error( "Renderable construction attempt via uninitialized Mesh!" );
		}
#endif // _DEBUG || _EDITOR
	}

	Renderable::~Renderable()
	{
	}

	const Matrix4x4* Renderable::WorldMatrix()
	{
		if( world_matrix )
			return world_matrix;

		if( transform )
			return &transform->GetFinalMatrix();

		return nullptr;
	}

	Vector3 Renderable::WorldPosition() const
	{
		if( world_matrix )
			return world_matrix->GetRow< 3 >( 3 /* Last row holds the translation in row-major form. */ );

		if( transform )
			return transform->GetTranslation();

		return Vector3{};
	}

	void Renderable::SetMesh( const Mesh* mesh )
	{
		this->mesh = mesh;
	}

	void Renderable::SetMaterial( Material* material )
	{
		this->material = material;
	}
}
