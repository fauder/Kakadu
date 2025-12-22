#pragma once

// Engine Includes.
#include "Material.hpp"
#include "Mesh.h"
#include "Scene/Transform.h"

namespace Engine
{
	class Renderer;

	class Renderable
	{
		friend class Renderer;

	public:
		Renderable();

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Renderable );

		/* Shadows are off by default, for perf. reasons. User must enable them explicitly in order to use them. */
		Renderable( const Mesh* mesh, Material* material, Transform* transform = nullptr, const bool receive_shadows = false, const bool cast_shadows = false );

		~Renderable();

	/* Get/Set: */
		const Transform*	GetTransform()	const { return transform;	}
		const Mesh*			GetMesh()		const { return mesh;		}
		const Material*		GetMaterial()	const { return material;	}

		void SetMesh( const Mesh* mesh );
		void SetMaterial( Material* material );

	/* Queries: */
		bool IsEnabled()			const { return is_enabled; }
		bool IsReceivingShadows()	const { return is_receiving_shadows; }
		bool IsCastingShadows()		const { return is_casting_shadows; }

	/* Toggling: */
		void Enable();
		void Disable();
		void ToggleOnOff();
		void ToggleOnOff( const bool enable );
		void ToggleShadowReceiving( const bool enable );
		void ToggleShadowCasting( const bool enable );

	private:
		Transform* transform;
		const Mesh* mesh;
		Material* material;

		bool is_enabled;
		bool is_receiving_shadows;
		bool is_casting_shadows;
		//bool padding[ 5 ];
	};
}
