#pragma once

// Engine Includes.
#include "Mesh.h"
#include "Texture.h"
#include "RHI/Usage.h"

// std Includes
#include <optional>
#include <vector>

namespace Kakadu
{
	template< Concepts::NotPointer AssetType >
	class AssetDatabase;

	class Model
	{
	public:
		using SizeType = i32;

		struct ImportSettings
		{
			RHI::Usage usage = RHI::Usage::StaticDraw;
		};

		static constexpr ImportSettings DEFAULT_IMPORT_SETTINGS = {};

		/* Maps to a glTF "primitive".
		 * It is a mesh with a unique material, i.e., a distinct draw call (unique in local MeshGroup, or glTF "mesh", not the whole model). */
		struct SubMesh
		{
			std::string name;
			Mesh& mesh; // Actual mesh storage is kept in the Model class.
			
			Texture* texture_albedo;
			Texture* texture_normal;
			std::optional< Color3 > color_albedo;
		};

		/* Maps to a glTF "mesh". 
		 * Does not containt a mesh directly; SubMeshes contained inside contain the actual Meshes.
		 * So basically, this is just a group. */
		struct MeshGroup
		{
			std::string name;
			std::vector< SubMesh > sub_meshes;
		};

		/* Same as a glTF "node". */
		struct Node
		{
			Node() = default;
			Node( const std::string& name, const Matrix4x4& transform_local, MeshGroup* mesh_group );

			DELETE_COPY_CONSTRUCTORS( Node );

			Node( Node&& );
			Node& operator =( Node&& );

			std::vector< i32 > children;

			std::string name;
			Matrix4x4 transform_local;
			MeshGroup* mesh_group;
		};

	private:
		ASSET_LOADER_CLASS_DECLARATION_FROM_FILE_ONLY( Model );

		friend class AssetDatabase< Model >;
		friend class Loader;

	public:
		Model();
		Model( const std::string& name );

		DELETE_COPY_CONSTRUCTORS( Model );
		DEFAULT_MOVE_CONSTRUCTORS( Model );

		~Model();

	/* Queries: */
		const std::string& Name() const { return name; }

		i32 NodeCount()			const { return ( i32 )nodes.size(); }
		i32 MeshCount()			const { return ( i32 )meshes.size(); }
		i32 MeshInstanceCount()	const { return mesh_instance_count; }
		i32 MeshGroupCount()	const { return ( i32 )mesh_groups.size(); }
		
		const std::vector< std::size_t >& TopLevelNodeIndices() const { return node_indices_top_level; }

		const std::vector< Node		>& Nodes()		const { return nodes; }
		const std::vector< Mesh		>& Meshes()		const { return meshes; }
		const std::vector< Texture*	>& Textures()	const { return textures; }

	private:
		std::string name;

		std::vector< Node		> nodes;
		std::vector< MeshGroup	> mesh_groups;
		std::vector< Mesh		> meshes;
		std::vector< Texture*	> textures; // Storage of textures is kept by the AssetDatabase< Texture >.

		std::vector< std::size_t > node_indices_top_level;

		i32 mesh_instance_count;
		/* 4 bytes of padding. */;
	};
}
