#pragma once
#include <string>

namespace PrCore::PathUtils
{
	void             Sanitize(char* p_path);
	std::string      Sanitize(std::string_view p_path);

	bool             IsAbsolute(const char* p_path);
	bool             IsRelative(const char* p_path);

	std::string      GetExtension(std::string_view p_path);
	std::string_view GetExtensionInPlace(std::string_view p_path);
	std::string      ReplaceExtension(std::string_view p_path, std::string_view p_ext);
	std::string      RemoveExtension(std::string_view p_path);
	std::string_view RemoveExtensionInPlace(std::string_view p_path);

	std::string      GetFilename(std::string_view p_path);
	std::string_view GetFilenameInPlace(std::string_view p_path);
	std::string      ReplaceFilename(std::string_view p_path, std::string_view p_name);
	std::string      RemoveFilename(std::string_view p_path);

	std::string      GetFile(std::string_view p_path);
	std::string_view GetFileInPlace(std::string_view p_path);
	std::string      PrReplaceFile(std::string_view p_path, std::string_view p_file);
	std::string      RemoveFile(std::string_view p_path);
	std::string_view RemoveFileInPlace(std::string_view p_path);

	std::string      MakePath(std::string_view p_dir, std::string_view p_file);
	std::string      MakePath(std::string_view p_dir, std::string_view p_file, std::string_view p_ext);

	std::vector<std::string>      SplitPath(std::string_view p_path);
	std::vector<std::string_view> SplitPathInPlace(std::string_view p_path);

	std::string                   GetSubFolder(std::string_view p_path, size_t generation = 0);
	std::string_view              GetSubFolderInPlace(std::string_view p_path, size_t generation = 0);

	std::string                   RemoveSubFolder(std::string_view p_path, size_t generation = 1);
	std::string_view              RemoveSubFolderInPlace(std::string_view p_path, size_t generation = 1);
}