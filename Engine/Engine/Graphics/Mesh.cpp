// Engine Includes.
#include "Mesh.h"
#include "Asset/Shader/_Attributes.glsl"

namespace Engine
{
	Mesh::Mesh()
		:
		primitive_type( PrimitiveType::Triangles )
	{}

	Mesh::Mesh( const std::span< const Vector3			> positions, 
				const std::string&						  name, 
				const std::span< const Vector3			> normals,
				const std::span< const Vector2			> uvs, 
				const std::span< const std::uint32_t	> indices, 
				const std::span< const Vector3			> tangents, 
				const PrimitiveType						  primitive_type, 
				const GLenum							  usage )
		:
		name( name ),
		indices( indices.begin(), indices.end() ),
		positions( positions.begin(), positions.end() ),
		normals( normals.begin(), normals.end() ),
		tangents( tangents.begin(), tangents.end() ),
		uvs( uvs.begin(), uvs.end() ),
		primitive_type( primitive_type ),
		instance_count( 1 )
	{
#ifdef _DEBUG
		if( normals.size() != tangents.size() )
			ServiceLocator< GLLogger >::Get().Warning( "Mesh \"" + name + "\": Only one of the Normals & Tangents were provided. This is most likely a mistake." );
#endif // _DEBUG

		unsigned int vertex_count_interleaved;
		const auto interleaved_vertices = MeshUtility::Interleave( vertex_count_interleaved, positions, normals, uvs, tangents );

		vertex_buffer = VertexBuffer( vertex_count_interleaved, std::span( interleaved_vertices ), name + " Vertex Buffer", usage );
		vertex_layout = VertexLayout( GatherAttributes( positions, normals, uvs, tangents ) );
		index_buffer  = indices.empty() ? std::nullopt : std::optional< IndexBuffer >( std::in_place, std::span( indices ), name + " Index Buffer", usage );
		vertex_array  = VertexArray( vertex_buffer, vertex_layout, index_buffer, name + " VAO");
	}

	Mesh::Mesh( std::vector< Vector3 >&&		positions,
				const std::string&				name,
				std::vector< Vector3 >&&		normals,
				std::vector< Vector2 >&&		uvs,
				std::vector< std::uint32_t >&&	indices,
				std::vector< Vector3 >&&		tangents,
				const PrimitiveType				primitive_type,
				const GLenum					usage )
		:
		name( name ),
		indices( indices ),
		positions( positions ),
		normals( normals ),
		tangents( tangents ),
		uvs( uvs ),
		primitive_type( primitive_type ),
		instance_count( 1 )
	{
		unsigned int vertex_count_interleaved;
		const auto interleaved_vertices = MeshUtility::Interleave( vertex_count_interleaved, positions, normals, uvs, tangents );

		vertex_buffer = VertexBuffer( vertex_count_interleaved, std::span( interleaved_vertices ), name + " Vertex Buffer", usage );
		vertex_layout = VertexLayout( GatherAttributes( positions, normals, uvs, tangents ) );
		index_buffer  = indices.empty() ? std::nullopt : std::optional< IndexBuffer >( std::in_place, std::span( indices ), name + " Index Buffer", usage );
		vertex_array  = VertexArray( vertex_buffer, vertex_layout, index_buffer, name + " VAO");
	}

	Mesh::Mesh( const Mesh& other,
				const std::initializer_list< VertexInstanceAttribute > instanced_attributes,
				const std::vector< float >& instance_data,
				const int instance_count,
				const GLenum instance_buffer_usage )
		:
		name( other.name + " (instanced)" ),
		indices( other.indices ),
		positions( other.positions ),
		normals( other.normals ),
		tangents( other.tangents ),
		uvs( other.uvs ),
		primitive_type( other.primitive_type ),
		instance_count( instance_count ),
		vertex_buffer( other.vertex_buffer ),
		vertex_layout( other.vertex_layout ),
		index_buffer( other.index_buffer )
	{
		for( auto instanced_attribute_iterator = instanced_attributes.begin(); instanced_attribute_iterator != instanced_attributes.end(); instanced_attribute_iterator++ )
			vertex_layout.Push( *instanced_attribute_iterator );

		instance_buffer = std::optional< InstanceBuffer >( std::in_place, instance_count, std::span( instance_data ), name + " Instance Buffer", instance_buffer_usage );

		vertex_array = VertexArray( vertex_buffer, vertex_layout, index_buffer, *instance_buffer, name + " VAO (instanced)");
	}

	Mesh::~Mesh()
	{}

	void Mesh::Update( const void* data ) const
	{
		vertex_buffer.Update( data );
	}

	void Mesh::Update_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
	{
		vertex_buffer.Update_Partial( data_span, offset_from_buffer_start );
	}

	void Mesh::UpdateInstanceData( const void* data ) const
	{
		ASSERT_DEBUG_ONLY( instance_buffer && "UpdateInstanceData() called on non-instanced Mesh!" );

		instance_buffer->Update( data );
	}

	void Mesh::UpdateInstanceData_Partial( const std::span< std::byte > data_span, const std::size_t offset_from_buffer_start ) const
	{
		ASSERT_DEBUG_ONLY( instance_buffer && "UpdateInstanceData_Partial() called on non-instanced Mesh!" );

		instance_buffer->Update_Partial( data_span, offset_from_buffer_start );
	}

	std::array< VertexAttribute, 4 > Mesh::GatherAttributes( const std::span< const Vector3 >& positions,
															 const std::span< const Vector3 >& normals,
															 const std::span< const Vector2 >& uvs,
															 const std::span< const Vector3 >& tangents )
	{
		auto CountOf = []( auto&& attribute_container ) { return attribute_container.empty() ? 0 : int( sizeof( attribute_container.front() ) / sizeof( float ) ); };

		constexpr bool is_instanced = false;

		return std::array< VertexAttribute, 4 >
		( {
			VertexAttribute{ CountOf( positions ),		GL_FLOAT,	is_instanced, POSITION_LOCATION		},
			VertexAttribute{ CountOf( normals ),		GL_FLOAT,	is_instanced, NORMAL_LOCATION		},
			VertexAttribute{ CountOf( uvs ),			GL_FLOAT,	is_instanced, TEXCOORDS_LOCATION	},
			VertexAttribute{ CountOf( tangents ),		GL_FLOAT,	is_instanced, TANGENT_LOCATION		},
		} );
	}
}
