#include "Core/Common/pearl_pch.h"

#include "Core/Filesystem/FileSystemNew.h"
#include "physfs/physfs.h"

using namespace PrCore::File;
	
PHYSFS_File* ToPhys(FileHandle p_handle)
{
	return reinterpret_cast<PHYSFS_File*>(p_handle);
}

FileSystemNew::FileSystemNew()
{
	PHYSFS_init(NULL);
}

FileSystemNew::~FileSystemNew()
{
	PHYSFS_deinit();
}

void PrCore::File::FileSystemNew::PrintError()
{
	PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
	if (errorCode != PHYSFS_ErrorCode::PHYSFS_ERR_OK)
	{
		auto error = PHYSFS_getErrorByCode(errorCode);
		PRLOG_ERROR("Filesystem error: {}", error);
	}
}

void FileSystemNew::MountDir(std::string_view p_path, std::string_view p_mountPoint)
{
	if(!PHYSFS_mount(p_path.data(), p_mountPoint.data(), 1))
		PrintError();
}

void FileSystemNew::UnmountDir(std::string_view p_path)
{
	if(!PHYSFS_unmount(p_path.data()))
		PrintError();
}

void FileSystemNew::OpenArchive(std::string_view p_path, std::string_view p_mountPoint)
{
	if(!PHYSFS_mount(p_path.data(), p_mountPoint.data(), 1))
		PrintError();
}

void FileSystemNew::CloseArchive(std::string_view p_path)
{
	if(!PHYSFS_unmount(p_path.data()))
		PrintError();
}

void FileSystemNew::SetWriteDir(std::string_view p_path)
{
	if(!PHYSFS_setWriteDir(p_path.data()))
		PrintError();
}

std::string_view PrCore::File::FileSystemNew::GetWriteDir()
{
	return PHYSFS_getWriteDir();
}

const std::vector<std::string_view> FileSystemNew::GetMountPaths()
{
	std::vector<std::string_view> retVec;
	for (auto it = PHYSFS_getSearchPath(); it != NULL; ++it)
	{
		if (*it != nullptr)
			retVec.push_back(*it);
	}

	return retVec;
}

void FileSystemNew::CreateDir(std::string_view p_path)
{
	if(!PHYSFS_mkdir(p_path.data()))
		PrintError();
}

void FileSystemNew::DeleteDir(std::string_view p_path)
{
	if(!PHYSFS_delete(p_path.data()))
		PrintError();
}

bool PrCore::File::FileSystemNew::IsDir(std::string_view p_path)
{
	PHYSFS_Stat stat;
	if (!PHYSFS_stat(p_path.data(), &stat))
	{
		PrintError();
		return false;
	}

	return stat.filetype == PHYSFS_FILETYPE_DIRECTORY;
}

void PrCore::File::FileSystemNew::FileDelete(std::string_view p_path)
{
	if(!PHYSFS_delete(p_path.data()))
		PrintError();
}

bool PrCore::File::FileSystemNew::FileExist(std::string_view p_path)
{
	return PHYSFS_exists(p_path.data());
}

bool PrCore::File::FileSystemNew::IsFile(std::string_view p_path)
{
	PHYSFS_Stat stat;
	if (!PHYSFS_stat(p_path.data(), &stat))
	{
		PrintError();
		return false;
	}

	return stat.filetype == PHYSFS_FILETYPE_REGULAR;
}

FileStats FileSystemNew::GetStat(std::string_view p_path)
{
	PHYSFS_Stat physStat;
	if (!PHYSFS_stat(p_path.data(), &physStat))
	{
		PrintError();
		return FileStats();
	}
	else
	{
		FileStats prStat;
		prStat.accessTime = physStat.accesstime;
		prStat.createTime = physStat.createtime;
		prStat.fileSize = physStat.filesize;
		prStat.modTime = physStat.modtime;
		prStat.readOnly = physStat.readonly;
		prStat.type = static_cast<FileType>(physStat.filetype);

		return prStat;
	}
}

std::vector<std::string> PrCore::File::FileSystemNew::EnumerateFiles(std::string_view p_path)
{
	char** rc = PHYSFS_enumerateFiles(p_path.data());
	char** i;
	
	if (rc == nullptr)
		return std::vector<std::string>();

	std::vector<std::string> retVec;
	for (auto i = rc; *i != NULL; i++)
	{
		retVec.push_back(*i);
	}

	PHYSFS_freeList(rc);
	return retVec;
}

std::string_view PrCore::File::FileSystemNew::GetExecutablePath()
{
	return PHYSFS_getBaseDir();
}

PrFilePtr PrCore::File::FileSystemNew::OpenFileWrapper(std::string_view p_path)
{
	FileHandle handle = FileOpen(p_path, OpenMode::Read);
	if (handle == nullptr)
		return nullptr;

	auto file = std::make_shared<PrFile>(handle, std::string_view{ p_path });
	return file;
}

FileHandle PrCore::File::FileSystemNew::FileOpen(std::string_view p_path, OpenMode p_openMode)
{
	FileHandle handle;
	if (p_openMode == OpenMode::Append)
	{
		handle = PHYSFS_openAppend(p_path.data());
	}
	else if (p_openMode == OpenMode::Write)
	{
		handle = PHYSFS_openWrite(p_path.data());
	}
	else if (p_openMode == OpenMode::Read)
	{
		handle = PHYSFS_openRead(p_path.data());
	}
	else
	{
		PRLOG_ERROR("Cannot open file {}, invalid openMode", p_path);
		return FileHandle{};
	}

	if (handle == nullptr)
		PrintError();

	return handle;
}

void PrCore::File::FileSystemNew::FileClose(FileHandle p_handle)
{
	if(!PHYSFS_close(ToPhys(p_handle)))
		PrintError();
}

size_t PrCore::File::FileSystemNew::FileRead(FileHandle p_handle, void* p_buffer, size_t p_length)
{
	PR_ASSERT(p_buffer, "Buffer is invalid");

	int readBytes = PHYSFS_readBytes(ToPhys(p_handle), p_buffer, p_length);
	if(readBytes == -1)
		PrintError();
	
	return readBytes;
}

size_t PrCore::File::FileSystemNew::FileWrite(FileHandle p_handle, const void* p_buffer, size_t p_length)
{
	PR_ASSERT(p_buffer, "Buffer is invalid");

	int writeBytes = PHYSFS_writeBytes(ToPhys(p_handle), p_buffer, p_length);
	if (writeBytes == -1)
		PrintError();
	
	return writeBytes;
}

int PrCore::File::FileSystemNew::FileSeek(FileHandle p_handle, size_t p_pos)
{
	int error = PHYSFS_seek(ToPhys(p_handle), p_pos);
	if(!error)
		PrintError();

	return error;
}

int PrCore::File::FileSystemNew::FileTell(FileHandle p_handle)
{
	int pos = PHYSFS_tell(ToPhys(p_handle));
	if (pos == -1)
		PrintError();

	return pos;
}

int PrCore::File::FileSystemNew::FileSize(FileHandle p_handle)
{
	int size = PHYSFS_fileLength(ToPhys(p_handle));
	if (size == -1)
		PrintError();

	return size;
}

bool PrCore::File::FileSystemNew::FileEOF(FileHandle p_handle)
{
	return PHYSFS_eof(ToPhys(p_handle));
}

bool PrCore::File::FileSystemNew::FileSetBuffer(FileHandle p_handle, size_t p_bufferSize)
{
	int error = PHYSFS_setBuffer(ToPhys(p_handle), p_bufferSize);
	if (!error)
		PrintError();

	return error;
}

bool PrCore::File::FileSystemNew::FileFlush(FileHandle p_handle)
{
	int error = PHYSFS_flush(ToPhys(p_handle));
	if (!error)
		PrintError();

	return error;
}
