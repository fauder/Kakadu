#pragma once

// std Includes.
#include <cstddef> // std::byte.

#define BUFFER_OFFSET( idx ) ( static_cast< std::byte* >( 0 ) + ( idx ) )

#define ASSET_LOADER_CLASS_DECLARATION_FROM_FILE_ONLY( class_name ) \
class Loader\
{\
public:\
	Loader( const Loader& ) = delete;\
	Loader& operator =( const Loader& ) = delete;\
	static std::optional< class_name > FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings );\
	static std::optional< class_name > FromFile( const std::string_view name, const std::initializer_list< std::string > file_paths, const ImportSettings& import_settings );\
private:\
	Loader()\
	{}\
	static Loader& Instance()\
	{\
		static Loader instance;\
		return instance;\
	}\
};

#define ASSET_LOADER_CLASS_DECLARATION( class_name ) \
class Loader\
{\
public:\
	Loader( const Loader& ) = delete;\
	Loader& operator =( const Loader& ) = delete;\
	static std::optional< class_name > FromFile( const std::string_view name, const std::string& file_path, const ImportSettings& import_settings );\
	static std::optional< class_name > FromFile( const std::string_view name, const std::initializer_list< std::string > file_paths, const ImportSettings& import_settings );\
	static std::optional< class_name > FromMemory( const std::string_view name, const std::byte* data, const int size, const bool data_is_raw_bytes_instead_of_file_contents, const ImportSettings& import_settings );\
private:\
	Loader()\
	{}\
	static Loader& Instance()\
	{\
		static Loader instance;\
		return instance;\
	}\
};
