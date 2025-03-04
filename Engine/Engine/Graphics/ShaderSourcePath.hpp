// Engine Includes.
#include "ShaderType.h"

// std Include.
#include <filesystem>

namespace Engine
{
	template< ShaderType Type > 
	class ShaderSourcePath
	{
	public:
		constexpr ShaderSourcePath()
			:
			source_path( nullptr ),
			size( 0u )
		{}

		constexpr explicit ShaderSourcePath( const char* source_path, const std::size_t size )
			:
			source_path( source_path ),
			size( size )
		{}

		constexpr explicit ShaderSourcePath( const std::string& source_path )
			:
			source_path( source_path.c_str() ),
			size( source_path.size() )
		{}

		constexpr explicit ShaderSourcePath( const std::string_view source_path )
			:
			source_path( source_path ),
			size( source_path.size() )
		{}

		template< std::size_t Size >
		constexpr ShaderSourcePath( const std::array< char, Size > source_path_array )
			:
			source_path( source_path_array.data() ),
			size( source_path_array.size() )
		{}

		CONSTEXPR_DEFAULT_COPY_AND_MOVE_CONSTRUCTORS( ShaderSourcePath );

		constexpr ~ShaderSourcePath()
		{}

		constexpr operator const char* ( )			const { return source_path; }
		constexpr operator std::string_view()		const { return std::string_view( source_path, size ); }
		constexpr operator std::filesystem::path()	const { return source_path; }

		constexpr bool Empty()		 const { return size == 0u; }
		constexpr std::size_t Size() const { return size; }

	private:
		const char* source_path;
		std::size_t size;
	};

	using   VertexShaderSourcePath = ShaderSourcePath< ShaderType::Vertex   >;
	using FragmentShaderSourcePath = ShaderSourcePath< ShaderType::Fragment >;
	using GeometryShaderSourcePath = ShaderSourcePath< ShaderType::Geometry >;

	namespace Literals
	{
		constexpr VertexShaderSourcePath operator"" _vert( const char* source_path, std::size_t size )
		{
			return VertexShaderSourcePath{ source_path, size };
		}

		constexpr FragmentShaderSourcePath operator"" _frag( const char* source_path, std::size_t size )
		{
			return FragmentShaderSourcePath{ source_path, size };
		}

		constexpr GeometryShaderSourcePath operator"" _geom( const char* source_path, std::size_t size )
		{
			return GeometryShaderSourcePath{ source_path, size };
		}
	}
}
