#pragma once

#include "FileWrapper.h"

#include "Core/Utils/Singleton.h"
#include <string>

namespace PrCore::File {

	enum OpenMode : uint8_t
	{
		Append     = 1,
		Read       = 2,
		Write      = 4,
		Invalid    = 0,
	};

	class FileSystemNew : public Utils::Singleton<FileSystemNew> {
	public:
		// Mounts the directory to the seach paths, last opened has the highest priority
		void MountDir(const char* p_path, const char* p_mountPoint = NULL);
		// Unmounts the directory from the seach path
		void UnmountDir(const char* p_path);

		// To be used later
		void OpenArchive(const char* p_path, const char* p_mountPoint = NULL);
		void CloseArchive(const char* p_path);

		// Writing directory is a directory where file system can write files
		void                   SetWriteDir(const char* p_path);
		std::string_view       GetWriteDir();
		
		const std::vector<std::string_view> GetMountPaths();

		void     CreateDir(const char* p_path);
		void     DeleteDir(const char* p_path);
		bool     IsDir(const char* p_path);

		void     FileDelete(const char* p_path);
		bool     FileExist(const char* p_path);
		bool     IsFile(const char* p_path);

		FileStats                 GetStat(const char* p_path);

		// Enumerates files and dirs inside the p_path directory
		std::vector<std::string>  EnumerateFiles(const char* p_path = "/");

		// Main Engine Paths
		void              SetEngineRoot(const char* p_root) { m_engineRoot = p_root; }
		std::string_view  GetEngineRoot() { return m_engineRoot; }

		void              SetEngineAssetsPath(const char* p_assetPath) { m_engineAssets = p_assetPath; }
		std::string_view  GetEngineAssetsPath() { return m_engineAssets; }

		void              SetGameAssetsPath(const char* p_assetPath) { m_gameAssets = p_assetPath; }
		std::string_view  GetGameAssetsPath() { return m_gameAssets; }

		std::string_view  GetExecutablePath();

		// File manipulations

		// Opens read only file handle wrapper to easier read contents 
		PrFilePtr    OpenFileWrapper(const char* p_path);

		// Opens write/read file returns handle, use below functions to read/write file
		FileHandle   FileOpen(const char* p_path, OpenMode p_openMode = OpenMode::Read);
		void         FileClose(FileHandle p_handle);

		size_t       FileRead(FileHandle p_handle, void* p_buffer, size_t p_length);
		size_t       FileWrite(FileHandle p_handle, const void* p_buffer, size_t p_length);

		int          FileSeek(FileHandle p_handle, size_t p_pos);
		int          FileTell(FileHandle p_handle);
		int          FileSize(FileHandle p_handle);
		bool         FileEOF(FileHandle p_handle);

		bool         FileSetBuffer(FileHandle p_handle, size_t p_bufferSize);
		bool         FileFlush(FileHandle p_handle);

	private:
		FileSystemNew();
		~FileSystemNew();

		void PrintError();

		std::string  m_engineRoot;
		std::string  m_engineAssets;
		std::string  m_gameAssets;
		
		friend Utils::Singleton<FileSystemNew>;
	};
}