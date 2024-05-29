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
		return std::string(p_path.substr(p_path.find_last_of('.')));
	}

	std::string_view GetExtensionInPlace(std::string_view p_path)
	{
		return p_path.substr(p_path.find_last_of('.'));
	}

	std::string GetFilename(std::string_view p_path)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		const size_t end = p_path.find_last_of(".");
		return std::string(p_path.substr(begin, end - begin));
	}

	std::string_view GetFilenameInPlace(std::string_view p_path)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		const size_t end = p_path.find_last_of(".");
		return p_path.substr(begin, end - begin);
	}

	std::string ReplaceFilename(std::string_view p_path, std::string_view p_name)
	{
		const size_t begin = p_path.find_last_of('/') + 1;
		const size_t end = p_path.find_last_of('.');

		const std::string_view path = p_path.substr(0, begin);
		const std::string_view extension = end == std::string_view::npos ? "" : p_path.substr(end);
		return std::string(path).append(p_name).append(extension);
	}

	std::string RemoveFilename(std::string_view p_path)
	{
		const size_t filenamePos = p_path.find_last_of('/');
		return std::string(p_path.substr(0, filenamePos));
	}

	std::string_view RemoveFilenameInPlace(std::string_view p_path)
	{
		const size_t filenamePos = p_path.find_last_of('/');
		return p_path.substr(0, filenamePos);
	}

	std::string ReplaceExtension(std::string_view p_path, std::string_view p_ext)
	{
		if (p_ext.front() == '.')
			p_ext.remove_prefix(1);

		const size_t extPos = p_path.find_last_of('.') + 1;
		if (extPos == std::string_view::npos)
		{
			return std::string(p_path).append(".").append(p_ext);
		}

		const std::string_view path = p_path.substr(0, extPos);
		return std::string(path).append(p_ext);
	}

	std::string RemoveExtension(std::string_view p_path)
	{
		const size_t extPos = p_path.find_last_of('.');
		return std::string(p_path.substr(0, extPos));
	}

	std::string_view RemoveExtensionInPlace(std::string_view p_path)
	{
		const size_t extPos = p_path.find_last_of('.');
		return p_path.substr(0, extPos);
	}

	std::string MakePath(std::string_view p_dir, std::string_view p_file)
	{
		std::string path;
		path.reserve(p_dir.length() + p_file.length() + 1);
		return path.append(p_dir).append("/").append(p_file);
	}

	std::string MakePath(std::string_view p_dir, std::string_view p_file, std::string_view p_ext)
	{
		std::string path;
		path.reserve(p_dir.length() + p_file.length() + p_ext.length() + 2);
		return path.append(p_dir).append("/").append(p_file).append(".").append(p_ext);
	}
	std::vector<std::string> SplitPath(std::string_view p_path)
	{
		std::vector<std::string> retVec;

		size_t beginPtr = 0;
		size_t endPtr = p_path.find_first_of('/:');
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
		size_t endPtr = p_path.find_first_of('/:');
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
		const auto subPath = RemoveFilenameInPlace(p_path);
		const auto subFolders = SplitPathInPlace(subPath);

		PR_ASSERT(generation < subFolders.size(), "Generation Index is bigger than subfolders count.");

		return std::string(subFolders[subFolders.size() - generation - 1]);
	}

	std::string_view GetSubFolderInPlace(std::string_view p_path, size_t generation)
	{
		const auto subPath = RemoveFilenameInPlace(p_path);
		const auto subFolders = SplitPathInPlace(subPath);

		PR_ASSERT(generation < subFolders.size(), "Generation Index is bigger than subfolders count.");

		return subFolders[subFolders.size() - generation - 1];
	}
}