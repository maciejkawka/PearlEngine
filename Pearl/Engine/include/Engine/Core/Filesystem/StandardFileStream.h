#pragma once
#include"DataStream.h"
#include<fstream>

namespace PrCore::Filesystem {

	class StandardFileStream : public DataStream {
	public:
		StandardFileStream(std::fstream* p_fstream);

		StandardFileStream(std::fstream* p_fstream, const std::string& p_name, DataAccess p_dataAccess = DataAccess::Both);

		StandardFileStream(std::fstream* p_fstream, DataAccess p_dataAccess);

		virtual ~StandardFileStream();

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
		std::fstream* m_fileStream;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<StandardFileStream> StandardFileStreamPtr;
}