#pragma once
#include"DataStream.h"
#include<memory>

class PHYSFS_File;
typedef PHYSFS_File PrFile;

namespace PrCore::Filesystem {

	class FileStream : public DataStream {
	public:
		FileStream(PrFile* p_file, const std::string& p_name, DataAccess p_dataAccess = DataAccess::Read);

		FileStream(PrFile* p_file, DataAccess p_dataAccess = DataAccess::Read);

		virtual ~FileStream();

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
		PrFile* m_file;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<FileStream> FileStreamPtr;
}