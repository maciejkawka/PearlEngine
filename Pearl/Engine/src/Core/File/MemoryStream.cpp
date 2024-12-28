#include"Core/Common/pearl_pch.h"

#include"Core/File/MemoryStream.h"

using namespace PrCore::File;

MemoryStream::MemoryStream(void* p_data, size_t p_size) :
	DataStream(),
	m_pointerPos(0)
{
	m_data = static_cast<unsigned char*>(p_data);
	m_size = p_size;
}

MemoryStream::MemoryStream(void* p_data, size_t p_size, const std::string& p_name, DataAccess p_dataAccess) :
	DataStream(p_name, p_dataAccess),
	m_pointerPos(0)
{
	m_data = static_cast<unsigned char*>(p_data);
	m_size = p_size;

}

MemoryStream::MemoryStream(void* p_data, size_t p_size, DataAccess p_dataAccess) :
	DataStream(p_dataAccess),
	m_pointerPos(0)
{
	m_data = static_cast<unsigned char*>(p_data);
	m_size = p_size;

}

MemoryStream::MemoryStream(const std::string& p_name, size_t p_size, DataAccess p_dataAccess) :
	DataStream(p_name, p_dataAccess),
	m_pointerPos(0)
{
	m_size = p_size;
	m_autoDelete = true;
	m_data = new unsigned char[m_size];
}

MemoryStream::~MemoryStream()
{
	Close(m_autoDelete);
}

size_t MemoryStream::Read(void* p_data, size_t p_size)
{
	if (m_data == nullptr)
		return 0;

	if (m_dataAccess == DataAccess::Write)
	{
		PRLOG_WARN("File {0} cannot be read", m_name);
		return m_pointerPos;
	}

	if (p_size + m_pointerPos > m_size)
		return m_pointerPos;

	if (p_size == 0)
		p_size = m_size;

	std::memcpy(p_data, m_data + m_pointerPos, p_size);
	m_pointerPos += p_size;

	return m_pointerPos;
}

size_t MemoryStream::Write(const void* p_data, size_t p_size)
{
	if (m_data == nullptr)
		return 0;

	if (m_dataAccess == DataAccess::Read)
	{
		PRLOG_WARN("File {0} cannot be write", m_name);
		return m_pointerPos;
	}

	if (p_size + m_pointerPos > m_size)
		return m_pointerPos;

	std::memcpy(m_data + m_pointerPos, p_data, p_size);
	m_pointerPos += p_size;

	return m_pointerPos;
}

void MemoryStream::Clear()
{
	if (m_data == nullptr)
		return;
	if (m_dataAccess == DataAccess::Read)
		return;

	std::memset(m_data, 0, m_size);
}

void MemoryStream::Close(bool p_deleteStream)
{
	if (m_data == nullptr)
		return;

	Clear();
	if (p_deleteStream)
		delete m_data;
	m_data = nullptr;
}

std::string MemoryStream::GetLine(size_t p_dataSize, char delim)
{
	if (m_data == nullptr)
		return std::string();

	if (p_dataSize + m_pointerPos > m_size)
		p_dataSize = m_size - m_pointerPos;

	auto foundChar = std::memchr(m_data + m_pointerPos, (int)delim, p_dataSize);

	if (foundChar)
	{
		auto indexDiff = static_cast<unsigned char*>(foundChar) - (m_data + m_pointerPos);

		char* data = new char[indexDiff];
		std::memcpy(data, m_data + m_pointerPos, indexDiff);
		m_pointerPos += indexDiff;

		auto returnString = std::string(data, indexDiff);
		delete[] data;

		return returnString;
	}
	else
	{
		auto memorySize = m_size - m_pointerPos;
		char* data = new char[memorySize];
		std::memcpy(data, m_data + m_pointerPos, memorySize);
		m_pointerPos = m_size;

		auto returnString = std::string(data, memorySize);
		delete[] data;

		return  returnString;
	}
}

int MemoryStream::Peek()
{
	if (m_data == nullptr)
		return INT_MAX;

	if (m_pointerPos + 1 > m_size)
		return INT_MAX;

	return static_cast<int>(*(m_data + m_pointerPos + 1));
}

bool MemoryStream::End()
{
	return m_pointerPos == m_size;
}

void MemoryStream::Seek(size_t p_newPosition)
{
	if (m_data == nullptr)
		return;

	if (p_newPosition > m_size)
		return;

	m_pointerPos = p_newPosition;
}

size_t MemoryStream::Tell()
{
	return m_pointerPos;
}


