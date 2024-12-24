#pragma once

namespace PrCore::File {

	using FileHandle = void*;

	enum class FileType
	{
		RegularFile,
		Directory,
		Symlink,
		Other
	};

	struct FileStats
	{
		int fileSize = 0;
		int modTime = 0;
		int createTime = 0;
		int accessTime = 0;

		FileType type = FileType::Other;
		bool readOnly = false;
	};

	class PrFile {
	public:
		PrFile(FileHandle p_handle, std::string_view p_path) :
			m_handle(p_handle),
			m_path(p_path)
		{}

		~PrFile();

		size_t    Read(void* p_buffer, size_t p_length);
		int       Seek(size_t p_pos);
		int       Tell();
		bool      IsEof();

		int       GetSize();
		FileStats GetStat();

		const std::string& GetPath() const { return m_path; }

	private:
		void PrintError();

		FileHandle  m_handle;
		std::string m_path;
	};
	using PrFilePtr = std::shared_ptr<PrFile>;
}