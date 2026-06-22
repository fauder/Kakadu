#pragma once

// Engine Includes.
#include "Material.hpp"
#include "Mesh.h"
#include "Scene/Transform.h"

namespace Kakadu
{
	class Renderer;

	class Renderable
	{
		friend class Renderer;

	public:
		Renderable();

		DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( Renderable );

		/* Shadows are off by default, for perf. reasons. User must enable them explicitly in order to use them. 
		 *
		 * "world_matrix" is for hierarchy-baked nodes that supply a precomputed world matrix directly (which may contain shear and therefore can not be a Transform).
		 * When provided, it takes precedence over "transform" as the source of the world transform. Root/standalone objects keep using "transform" instead. */
		Renderable( const Mesh* mesh, Material* material, Transform* transform = nullptr, const bool receive_shadows = false, const bool cast_shadows = false, const Matrix4x4* world_matrix = nullptr );

		~Renderable();

	/* Get/Set: */
		const Transform*	GetTransform()	const { return transform;	}
		const Mesh*			GetMesh()		const { return mesh;		}
		const Material*		GetMaterial()	const { return material;	}

		/* World matrix to upload to the GPU.
		 * Sourced from the directly-supplied world matrix (hierarchy-baked nodes) or the Transform's final matrix (root objects).
		 * Returns nullptr if neither is set. */
		const Matrix4x4* WorldMatrix();
		/* Cheap world-space position that can be used for sorting. Does not trigger a Transform final-matrix recompute. */
		Vector3 WorldPosition() const;
		bool HasWorldTransform() const { return transform || world_matrix; }

		void SetMesh( const Mesh* mesh );
		void SetMaterial( Material* material );

	public:
		bool is_enabled;
		bool is_receiving_shadows;
		bool is_casting_shadows;
		/* 5 bytes of padding. */

	private:
		Transform* transform;
		const Matrix4x4* world_matrix;
		const Mesh* mesh;
		Material* material;
	};
}
