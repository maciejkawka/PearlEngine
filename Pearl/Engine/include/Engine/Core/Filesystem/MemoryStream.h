#pragma once
#include"Core/Filesystem/DataStream.h"

namespace PrCore::File {

	class MemoryStream : public DataStream {
	public:
		MemoryStream(void* p_data, size_t p_size);

		MemoryStream(void* p_data, size_t p_size, const std::string& p_name, DataAccess p_dataAccess = DataAccess::Both);

		MemoryStream(void* p_data, size_t p_size, DataAccess p_dataAccess);

		MemoryStream(const std::string& p_name, size_t p_size, DataAccess p_dataAccess = DataAccess::Both);

		virtual ~MemoryStream() override;

		virtual size_t Read(void* p_data, size_t p_size = 0) override;
		virtual size_t Write(const void* p_data, size_t p_size) override;
		virtual void Clear() override;
		virtual void Close(bool p_deleteStream = false) override;

		virtual std::string GetLine(size_t p_dataSize, char delim = '\n') override;
		virtual int Peek() override;

		virtual bool End() override;
		virtual void Seek(size_t p_newPosition) override;
		virtual size_t Tell() override;

	protected:
		unsigned char* m_data;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<MemoryStream> MemoryStreamPtr;
}