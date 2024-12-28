#pragma once
#include"DataStream.h"
#include<fstream>

namespace PrCore::File {

	class StandardFileStream : public DataStream {
	public:
		StandardFileStream(std::fstream* p_fstream);

		StandardFileStream(std::fstream* p_fstream, const std::string& p_name, DataAccess p_dataAccess = DataAccess::Both);

		StandardFileStream(std::fstream* p_fstream, DataAccess p_dataAccess);

		virtual ~StandardFileStream() override;

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
		std::fstream* m_fileStream;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<StandardFileStream> StandardFileStreamPtr;
}