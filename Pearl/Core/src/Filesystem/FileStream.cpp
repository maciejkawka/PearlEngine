#include"Core/Common/pearl_pch.h"

#include"Core/Filesystem/FileStream.h"
#include"physfs/physfs.h"

using namespace PrCore::Filesystem;

FileStream::FileStream(PrFile* p_file, DataAccess p_dataAccess) :
	DataStream(),
	m_file(p_file),
	m_pointerPos(0)
{
	m_size = PHYSFS_fileLength(p_file);
	m_dataAccess = p_dataAccess;
}

FileStream::FileStream(PrFile* p_file, const std::string& p_name, DataAccess p_dataAccess) :
	DataStream(p_name, p_dataAccess),
	m_file(p_file),
	m_pointerPos(0)
{
	m_size = PHYSFS_fileLength(p_file);
}

FileStream::~FileStream()
{
	Close(m_autoDelete);
}

size_t FileStream::Read(void* p_data, size_t p_size)
{
	if (m_dataAccess != DataAccess::Read)
	{
		PRLOG_WARN("File {0} cannot be read", m_name);
		return m_pointerPos;
	}

	if (p_size == 0)
		p_size = m_size;

	auto byteRead = PHYSFS_readBytes(m_file, p_data, p_size);
	m_pointerPos += byteRead;

	return m_pointerPos;
}

size_t FileStream::Write(const void* p_data, size_t p_size)
{
	if (m_dataAccess != DataAccess::Write)
	{
		PRLOG_WARN("File {0} cannot be read", m_name);
		return m_pointerPos;
	}

	auto byteWrite = PHYSFS_writeBytes(m_file, p_data, p_size);
	m_pointerPos = byteWrite;

	return m_pointerPos;
}

void FileStream::Clear()
{
}

void FileStream::Close(bool p_deleteStream)
{
	PHYSFS_close(m_file);
}

std::string FileStream::GetLine(size_t p_dataSize, char delim)
{
	if (m_dataAccess != DataAccess::Read)
		return std::string();

	char* data = new char[p_dataSize];
	auto dataRead = PHYSFS_readBytes(m_file, data, p_dataSize);

	std::string text(data, dataRead);
	auto findText = text.find_first_of(delim);

	if (findText == std::string::npos)
	{
		std::string returnString(data, dataRead);
		delete[] data;
		m_pointerPos += dataRead + 1;
		Seek(m_pointerPos);
		return returnString;
	}
	else
	{
		std::string returnString(data, findText);
		delete[] data;
		m_pointerPos += findText + 1;
		Seek(m_pointerPos);
		return returnString;
	}
}

int FileStream::Peek()
{
	if (m_pointerPos == m_size)
		return 0;
	char* data = new char[m_pointerPos + 1];
	auto bytesRead = PHYSFS_readBytes(m_file, data, m_pointerPos + 1);
	char returnChar = data[m_pointerPos];
	delete[] data;

	return returnChar;
}

bool FileStream::End()
{
	auto test = PHYSFS_eof(m_file);
	return m_pointerPos == m_size;
}

void FileStream::Seek(size_t p_newPosition)
{
	PHYSFS_seek(m_file, p_newPosition);
}

size_t FileStream::Tell()
{
	return PHYSFS_tell(m_file);
}


