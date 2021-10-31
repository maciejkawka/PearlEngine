#pragma once
#include"FileStream.h"
#include"StandardFileStream.h"
#include<vector>

namespace PrCore::Filesystem {
	
#define ROOT_DIR "C:/Users/Maciej/Current Projects/GitHub/PearlEngine/PearlEngine/Resources"
#define CONFIG_DIR "Config"

	//TODO Implement Commented Functions
	class FileSystem {
	public:
		FileSystem(FileSystem&) = delete;
		FileSystem(FileSystem&&) = delete;
		FileSystem& operator=(const FileSystem&) = delete;
		FileSystem& operator=(FileSystem&&) = delete;

		inline static FileSystem& GetInstance() { return *s_fileSystem; }
		static void Init();
		static void Terminate();

		PrFile* OpenFile(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		//PrFile* OpenFileDirect(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		FileStreamPtr OpenFileStream(const std::string& p_name, DataAccess p_access = DataAccess::Read);
		//FileStreamPtr OpenFileStreamDirect(const std::string& p_name, DataAccess p_access = DataAccess::Read);

		void CloseFile(PrFile* p_file);

		//StandardFileStreamPtr OpenFileStandard(const std::string& p_name, DataAccess p_access = DataAccess::Read);

		//StandardFileStreamPtr ToStandardFileStream(const FileStreamPtr& p_fileStream);
		//FileStreamPtr ToFileStream(const StandardFileStreamPtr& p_fileStream);

		void DeleteDir(const std::string& p_name);
		void DeleteFile(const std::string& p_name);

		void CreateDir(const std::string& p_name);

		unsigned int GetFileCount(const std::string& p_dir);
		std::vector<std::string> GetFileList(const std::string& p_dir);

	private:
		FileSystem();
		~FileSystem();

		void InitDir();

		static FileSystem* s_fileSystem;
	};
}