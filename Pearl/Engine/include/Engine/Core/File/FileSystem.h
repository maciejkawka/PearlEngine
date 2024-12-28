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

	// IMPORTANT!! FileSystem expects string_view paths to be NULL-terminated
	class FileSystem : public Utils::Singleton<FileSystem> {
	public:
		// Mounts the directory to the seach paths, last opened has the highest priority
		void MountDir(std::string_view p_path, std::string_view p_mountPoint = "/");
		
		// Unmounts the directory from the seach path
		void UnmountDir(std::string_view p_path);

		// To be used later, open and close zip archives, bind the zip to the mountPoint
		void OpenArchive(std::string_view p_path, std::string_view p_mountPoint = "/");
		void CloseArchive(std::string_view p_path);

		// Writing directory is a directory where file system can write files
		void                   SetWriteDir(std::string_view p_path);
		std::string_view       GetWriteDir();
		
		const std::vector<std::string_view> GetMountPaths();

		void     CreateDir(std::string_view p_path);
		void     DeleteDir(std::string_view p_path);
		bool     IsDir(std::string_view p_path);

		void     FileDelete(std::string_view p_path);
		bool     FileExist(std::string_view p_path);
		bool     IsFile(std::string_view p_path);

		FileStats                 GetStat(std::string_view p_path);

		// Enumerates files and dirs inside the p_path directory
		std::vector<std::string>  EnumerateFiles(std::string_view p_path = "/");

		// Main Engine Paths
		void              SetEngineRoot(std::string_view p_root) { m_engineRoot = p_root; }
		std::string_view  GetEngineRoot() { return m_engineRoot; }

		void              SetEngineAssetsPath(std::string_view p_assetPath) { m_engineAssets = p_assetPath; }
		std::string_view  GetEngineAssetsPath() { return m_engineAssets; }

		void              SetGameAssetsPath(std::string_view p_assetPath) { m_gameAssets = p_assetPath; }
		std::string_view  GetGameAssetsPath() { return m_gameAssets; }

		std::string_view  GetExecutablePath();

		// Opens read only file handle wrapper to easier read contents 
		PrFilePtr    OpenFileWrapper(std::string_view p_path);

		// Opens write/read file returns handle, use below functions to read/write file
		FileHandle   FileOpen(std::string_view p_path, OpenMode p_openMode = OpenMode::Read);
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
		FileSystem();
		~FileSystem();

		void PrintError();

		std::string  m_engineRoot;
		std::string  m_engineAssets;
		std::string  m_gameAssets;
		
		friend Utils::Singleton<FileSystem>;
	};
}