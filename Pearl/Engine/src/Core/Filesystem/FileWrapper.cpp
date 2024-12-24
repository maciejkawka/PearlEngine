#include "Core/Common/pearl_pch.h"

#include "Core/Filesystem/FileWrapper.h"

#include "physfs/physfs.h"

using namespace PrCore::File;

// Defined in the FileSystem.cpp
extern PHYSFS_File* ToPhys(FileHandle p_handle);

PrFile::~PrFile()
{
	PHYSFS_close(ToPhys(m_handle));
}

size_t PrCore::File::PrFile::Read(void* p_buffer, size_t p_length)
{
	PR_ASSERT(p_buffer, "Buffer is invalid");

	int readBytes = PHYSFS_readBytes(ToPhys(m_handle), p_buffer, p_length);
	if (readBytes == -1)
		PrintError();

	return readBytes;
}

int PrCore::File::PrFile::Seek(size_t p_pos)
{
	int error = PHYSFS_seek(ToPhys(m_handle), p_pos);
	if (!error)
		PrintError();

	return error;
}

int PrCore::File::PrFile::Tell()
{
	int pos = PHYSFS_tell(ToPhys(m_handle));
	if (pos == -1)
		PrintError();

	return pos;
}

bool PrCore::File::PrFile::IsEof()
{
	return PHYSFS_eof(ToPhys(m_handle));
}

int PrCore::File::PrFile::GetSize()
{
	int size = PHYSFS_fileLength(ToPhys(m_handle));
	if (size == -1)
		PrintError();

	return size;
}

FileStats PrCore::File::PrFile::GetStat()
{
	PHYSFS_Stat physStat;
	if (!PHYSFS_stat(m_path.c_str(), &physStat))
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

void PrCore::File::PrFile::PrintError()
{
	PHYSFS_ErrorCode errorCode = PHYSFS_getLastErrorCode();
	if (errorCode != PHYSFS_ErrorCode::PHYSFS_ERR_OK)
	{
		auto error = PHYSFS_getErrorByCode(errorCode);
		PRLOG_ERROR("Filesystem error: {}", error);
	}
}
