#pragma once
#include"Core/Filesystem/DataStream.h"

namespace PrCore::Filesystem {

	class MemoryStream : public DataStream {
	public:
		MemoryStream(void* p_data, size_t p_size);

		MemoryStream(void* p_data, size_t p_size, const std::string& p_name, DataAccess p_dataAccess = DataAccess::Both);

		MemoryStream(void* p_data, size_t p_size, DataAccess p_dataAccess);

		MemoryStream(const std::string& p_name, size_t p_size, DataAccess p_dataAccess = DataAccess::Both);

		virtual ~MemoryStream();

		virtual size_t Read(void* p_data, size_t p_size = 0);
		virtual size_t Write(const void* p_data, size_t p_size);
		virtual void Clear();
		virtual void Close(bool p_deleteStream = false);

		virtual std::string GetLine(size_t p_dataSize, char delim = '\n');
		virtual int Peek();

		virtual bool End();
		virtual void Seek(size_t p_newPosition);
		virtual size_t Tell();

	protected:
		unsigned char* m_data;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<MemoryStream> MemoryStreamPtr;
}