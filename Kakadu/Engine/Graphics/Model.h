#pragma once

// Engine Includes.
#include "Mesh.h"
#include "Math/Matrix.hpp"
#include "RHI/Texture.h"
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

		/* Maps to a glTF material entry. Holds surface properties used to construct a Material in ModelInstance. */
		struct MaterialInfo
		{
			std::string name;

			RHI::Texture*           texture_albedo = nullptr;
			RHI::Texture*           texture_normal = nullptr;
			std::optional< Color3 > color_albedo;
		};

		/* Maps to a glTF "primitive".
		 * It is a single draw call: a Mesh paired with a MaterialInfo index. */
		struct MeshInfo
		{
			std::string name;
			Mesh&       mesh; // Actual mesh storage is kept in the Model class.
			i32         material_info_index; // Index into the material_infos array. -1 if the primitive has no material.
		};

		/* Maps to a glTF "mesh".
		 * Does not contain a mesh directly; Contains MeshInfos, which themselves reference the actual Meshes.
		 * So basically, this is just a group. */
		struct MeshGroup
		{
			std::string name;
			std::vector< MeshInfo > mesh_infos;
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
		i32 MaterialInfoCount()	const { return ( i32 )material_infos.size(); }

		const std::vector< std::size_t >& TopLevelNodeIndices() const { return node_indices_top_level; }

		const std::vector< Node			>& Nodes()			const { return nodes; }
		const std::vector< Mesh			>& Meshes()			const { return meshes; }
		const std::vector< RHI::Texture*>& Textures()		const { return textures; }
		const std::vector< MaterialInfo	>& MaterialInfos()	const { return material_infos; }

	private:
		std::string name;

		std::vector< Node			> nodes;
		std::vector< MeshGroup		> mesh_groups;
		std::vector< MaterialInfo	> material_infos;
		std::vector< Mesh			> meshes;
		std::vector< RHI::Texture*	> textures; // Storage of textures is kept by the AssetDatabase< Texture >.

		std::vector< std::size_t > node_indices_top_level;

		i32 mesh_instance_count;
		/* 4 bytes of padding. */;
	};
}
