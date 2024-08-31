#include "Core/Common/pearl_pch.h"

#include "Core/Utils/PathUtils.h"

#include <Shlwapi.h>

namespace PrCore::PathUtils
{
	void Sanitize(char* p_path)
	{
		char* ptr = p_path;
		while (*ptr != '\0')
		{
			if (*ptr == '\\')
				*ptr = '/';
			else
				*ptr = static_cast<char>(std::tolower(static_cast<unsigned int>(*ptr)));

			ptr++;
		}
	}

	void Sanitize(const std::string& p_path)
	{
		Sanitize(p_path.c_str());
	}

	bool IsAbsolute(const char* p_path)
	{
		return !PathIsRelativeA(p_path);
	}

	bool IsRelative(const char* p_path)
	{
		return PathIsRelativeA(p_path);
	}

	std::string GetExtension(std::string_view p_path)
	{
		return std::string(GetExtensionInPlace(p_path));
	}

	std::string_view GetExtensionInPlace(std::string_view p_path)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		std::string_view file = p_path.substr(begin);
		if (file == ".." || file == "." || file.empty())
			return std::string_view();

		file.remove_prefix(1);
		const size_t length = file.find_last_of('.');
		if (file.find_last_of('.') == file.npos)
			return  std::string_view();

		return file.substr(length);
	}

	std::string GetFilename(std::string_view p_path)
	{
		return std::string(GetFilenameInPlace(p_path));
	}

	std::string_view GetFilenameInPlace(std::string_view p_path)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		std::string_view file = p_path.substr(begin);
		if (file == ".." || file == "." || file.empty())
			return file;

		file.remove_prefix(1);
		size_t length = file.find_last_of('.');

		if (length != file.npos)
			length++;

		return p_path.substr(begin, length);
	}

	std::string ReplaceFilename(std::string_view p_path, std::string_view p_name)
	{
		const size_t begin = p_path.find_last_of('/') + 1;

		const std::string_view path = p_path.substr(0, begin);
		const std::string_view extension = GetExtensionInPlace(p_path);
		return std::string(path).append(p_name).append(extension);
	}

	std::string RemoveFilename(std::string_view p_path)
	{
		return ReplaceFilename(p_path, "");
	}

	std::string GetFile(std::string_view p_path)
	{
		return std::string(GetFileInPlace(p_path));
	}

	std::string_view GetFileInPlace(std::string_view p_path)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		return p_path.substr(begin);
	}

	std::string PrReplaceFile(std::string_view p_path, std::string_view p_file)
	{
		const size_t filePos = p_path.find_last_of('/') + 1;
		const std::string_view path = p_path.substr(0, filePos);

		return std::string(path).append(p_file);
	}

	std::string RemoveFile(std::string_view p_path)
	{
		return PrReplaceFile(p_path, "");
	}

	std::string_view RemoveFileInPlace(std::string_view p_path)
	{
		const size_t filePos = p_path.find_last_of('/') + 1;
		return p_path.substr(0, filePos);
	}

	std::string ReplaceExtension(std::string_view p_path, std::string_view p_ext)
	{
		const size_t extPos = GetExtensionInPlace(p_path).length();
		const std::string_view path = p_path.substr(0, p_path.length() - extPos);

		if (!p_ext.empty() && p_ext.front() != '.')
			return std::string(path).append(".").append(p_ext);

		return std::string(path).append(p_ext);
	}

	std::string RemoveExtension(std::string_view p_path)
	{
		return ReplaceExtension(p_path, "");
	}

	std::string_view RemoveExtensionInPlace(std::string_view p_path)
	{
		const size_t extPos = GetExtensionInPlace(p_path).length();
		return p_path.substr(0, p_path.length() - extPos);
	}

	std::string MakePath(std::string_view p_dir, std::string_view p_file)
	{
		std::string path;
		path.reserve(p_dir.length() + p_file.length() + 1);

		if (!p_dir.empty() && p_dir.back() == '/')
			p_dir.remove_suffix(1);

		if (!p_file.empty() && p_file.front() == '/')
			p_file.remove_prefix(1);

		return path.append(p_dir).append("/").append(p_file);
	}

	std::string MakePath(std::string_view p_dir, std::string_view p_file, std::string_view p_ext)
	{
		if (!p_file.empty() && p_file.back() == '.')
			p_file.remove_suffix(1);

		std::string path = MakePath(p_dir, p_file);

		if (!p_ext.empty() && p_ext.front() == '.')
			p_ext.remove_prefix(1);

		return path.append(".").append(p_ext);
	}
	std::vector<std::string> SplitPath(std::string_view p_path)
	{
		std::vector<std::string> retVec;

		size_t beginPtr = 0;
		size_t endPtr = p_path.find_first_of("/:");
		while(endPtr != std::string_view::npos)
		{
			std::string subName{ p_path.substr(beginPtr, endPtr - beginPtr) };
			retVec.push_back(std::move(subName));

			beginPtr = endPtr + 1;
			endPtr = p_path.find_first_of('/', beginPtr);
		}

		std::string subName(p_path.substr(beginPtr));
		retVec.push_back(std::move(subName));
		return retVec;
	}

	std::vector<std::string_view> SplitPathInPlace(std::string_view p_path)
	{
		std::vector<std::string_view> retVec;

		size_t beginPtr = 0;
		size_t endPtr = p_path.find_first_of("/:");
		while (endPtr != std::string_view::npos)
		{
			retVec.push_back(p_path.substr(beginPtr, endPtr - beginPtr));

			beginPtr = endPtr + 1;
			endPtr = p_path.find_first_of('/', beginPtr);
		}

		retVec.push_back(p_path.substr(beginPtr));
		return retVec;
	}

	std::string GetSubFolder(std::string_view p_path, size_t generation)
	{
		const auto subFolders = SplitPathInPlace(p_path);

		PR_ASSERT(generation < subFolders.size() - 1, "Generation Index is bigger than subfolders count.");

		return std::string(subFolders[subFolders.size() - generation - 2]);
	}

	std::string_view GetSubFolderInPlace(std::string_view p_path, size_t generation)
	{
		const auto subPath = RemoveFileInPlace(p_path);
		const auto subFolders = SplitPathInPlace(subPath);

		PR_ASSERT(generation < subFolders.size() - 1, "Generation Index is bigger than subfolders count.");

		return subFolders[subFolders.size() - generation - 1];
	}
}