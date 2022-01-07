#include"Core/Common/pearl_pch.h"

#include"Core/Filesystem/FileSystem.h"
#include"physfs/physfs.h"

using namespace PrCore::Filesystem;

FileSystem* FileSystem::s_fileSystem = nullptr;

PrCore::Filesystem::FileSystem::FileSystem()
{
	PHYSFS_init(NULL);
	PRLOG_INFO("Init Filesystem");
	InitDir();
}

PrCore::Filesystem::FileSystem::~FileSystem()
{
	PHYSFS_deinit();
	PRLOG_INFO("Terminate Filesystem");
}

void PrCore::Filesystem::FileSystem::Init()
{
	if (s_fileSystem != nullptr)
		return;

	s_fileSystem = new FileSystem();
}

void PrCore::Filesystem::FileSystem::Terminate()
{
	if (s_fileSystem == nullptr)
		return;

	delete s_fileSystem;
}

PrFile* FileSystem::OpenFile(const std::string& p_name, DataAccess p_access)
{
	PrFile* returnFile = nullptr;
	if (p_access == DataAccess::Read)
		returnFile = PHYSFS_openRead(p_name.c_str());
	else if (p_access == DataAccess::Write)
		returnFile = PHYSFS_openWrite(p_name.c_str());
	
	if (!returnFile)
		PRLOG_WARN("Cannot open file: {0}", p_name);
	return returnFile;
}

FileStreamPtr FileSystem::OpenFileStream(const std::string& p_name, DataAccess p_access)
{
	PrFile* file = OpenFile(p_name, p_access);
	if (file == nullptr)
		return FileStreamPtr();

	FileStreamPtr fileStream = std::make_shared<FileStream>(file, p_access);
	return fileStream;

}

void PrCore::Filesystem::FileSystem::CloseFile(PrFile* p_file)
{
	int error = PHYSFS_close(p_file);
	if (error)
	{
		PRLOG_ERROR("File cannot be closed");
		return;
	}

}

void FileSystem::DeleteDir(const std::string& p_name)
{
	DeleteFile(p_name);
}

void FileSystem::DeleteFile(const std::string& p_name)
{
	PHYSFS_delete(p_name.c_str());
}

void FileSystem::CreateDir(const std::string& p_name)
{
	PHYSFS_mkdir(p_name.c_str());
}

unsigned int FileSystem::GetFileCount(const std::string& p_dir)
{
	auto fileEnumerator = PHYSFS_enumerateFiles(p_dir.c_str());

	unsigned int fileNumber = 0;
	for (auto i = fileEnumerator; *i != NULL; i++)
	{
		if (!PHYSFS_isDirectory(*i))
			fileNumber++;
	}

	return fileNumber;
}

std::vector<std::string> FileSystem::GetFileList(const std::string& p_dir)
{
	std::vector<std::string> fileList;
	auto fileEnumerator = PHYSFS_enumerateFiles(p_dir.c_str());

	unsigned int fileNumber = 0;
	for (auto i = fileEnumerator; *i != NULL; i++)
	{
		if (!PHYSFS_isDirectory(*i))
			fileList.emplace_back(*i);
	}

	return fileList;
}

void FileSystem::InitDir()
{
	PHYSFS_mount(ROOT_DIR, NULL, 0);
	PHYSFS_setWriteDir(ROOT_DIR);
}
