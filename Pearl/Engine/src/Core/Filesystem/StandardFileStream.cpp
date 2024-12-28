#include"Core/Common/pearl_pch.h"

#include"Core/Filesystem/StandardFileStream.h"

using namespace PrCore::File;

StandardFileStream::StandardFileStream(std::fstream* p_fstream) :
	DataStream(),
	m_fileStream(p_fstream),
	m_pointerPos(0)
{
	m_fileStream->seekg(0, std::ios_base::end);
	m_size = m_fileStream->tellg();
	m_fileStream->seekg(std::ios_base::beg);
}

StandardFileStream::StandardFileStream(std::fstream* p_fstream, const std::string& p_name, DataAccess p_dataAccess) :
	DataStream(p_name, p_dataAccess),
	m_fileStream(p_fstream),
	m_pointerPos(0)
{
	m_fileStream->seekg(0, std::ios_base::end);
	m_size = m_fileStream->tellg();
	m_fileStream->seekg(std::ios_base::beg);
}

StandardFileStream::StandardFileStream(std::fstream* p_fstream, DataAccess p_dataAccess) :
	DataStream(p_dataAccess),
	m_fileStream(p_fstream),
	m_pointerPos(0)
{
	m_fileStream->seekg(0, std::ios_base::end);
	m_size = m_fileStream->tellg();
	m_fileStream->seekg(std::ios_base::beg);
}

StandardFileStream::~StandardFileStream()
{
	Close(m_autoDelete);
}

size_t StandardFileStream::Read(void* p_data, size_t p_size)
{
	if (m_dataAccess == DataAccess::Write)
	{
		PRLOG_WARN("File {0} cannot be read", m_name);
		return m_pointerPos;
	}
		
	if (p_size == 0)
		p_size = m_size;

	m_fileStream->read(static_cast<char*>(p_data), p_size);

	return Tell();
}

size_t StandardFileStream::Write(const void* p_data, size_t p_size)
{
	if (m_dataAccess == DataAccess::Read)
	{
		PRLOG_WARN("File {0} cannot be write", m_name);
		return 0;
	}

	m_fileStream->write(static_cast<const char*>(p_data), p_size);

	auto position = Tell();
	if (m_size < position)
		m_size = position;

	return position;
}

void StandardFileStream::Clear()
{

}

void StandardFileStream::Close(bool p_deleteStream)
{
	m_fileStream->close();
	m_size = 0;
	m_name = "";
	m_pointerPos = 0;

	if (p_deleteStream)
		delete m_fileStream;
}

std::string StandardFileStream::GetLine(size_t p_dataSize, char delim)
{
	char* data = new char[p_dataSize];
	m_fileStream->getline(data, static_cast<std::streamsize>((uint64_t)p_dataSize + 1), delim);

	auto readSize = m_fileStream->gcount();
	m_pointerPos += (size_t)readSize;
	if (m_fileStream->fail() && End())
	{
		PRLOG_WARN("File {0} corrupted", m_name);
		m_fileStream->clear();
		return std::string();
	}

	m_fileStream->clear();
	return std::string(data, (size_t)readSize);
}

int StandardFileStream::Peek()
{
	return m_fileStream->peek();
}

bool StandardFileStream::End()
{
	return m_pointerPos >= m_size;
}

void StandardFileStream::Seek(size_t p_newPosition)
{
	m_fileStream->seekg(p_newPosition);
	if (m_fileStream->fail() || m_fileStream->bad())
	{
		PRLOG_WARN("File {0} corrupted", m_name);
		m_fileStream->seekg(m_pointerPos);
	}
	else
		m_pointerPos = p_newPosition;
}

size_t StandardFileStream::Tell()
{
	return m_fileStream->tellg();
}

