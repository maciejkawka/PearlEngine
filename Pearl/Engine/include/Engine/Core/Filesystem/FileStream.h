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
		virtual ~FileStream() override;

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
		PrFile* m_file;
		size_t m_pointerPos;
	};

	typedef std::shared_ptr<FileStream> FileStreamPtr;
}