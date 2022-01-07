#pragma once
#include<string>
#include<memory>

namespace PrCore::Filesystem {

	enum class DataAccess {
		Write,
		Read,
		Both
	};

	class DataStream {
	public:
		DataStream() :
			m_size(0),
			m_name(""),
			m_dataAccess(DataAccess::Read),
			m_autoDelete(false)
		{}

		DataStream(const std::string& p_name, DataAccess p_dataAccess = DataAccess::Read) :
			m_size(0),
			m_name(p_name),
			m_dataAccess(p_dataAccess),
			m_autoDelete(false)
		{}

		DataStream(DataAccess p_dataAccess) :
			m_size(0),
			m_name(""),
			m_dataAccess(p_dataAccess),
			m_autoDelete(false)
		{}

		virtual ~DataStream() {}

		inline void SetDataAccess(DataAccess p_dataAccess) { m_dataAccess = p_dataAccess; }
		inline DataAccess GetDataAccess() { return m_dataAccess; }

		inline std::string GetName() { return m_name; }
		inline void SetName(const std::string& p_name) { m_name = p_name; }

		inline size_t GetSize() { return m_size; }

		inline void SetAutoDelete(bool p_autoDelete) { m_autoDelete = p_autoDelete; }
		inline bool IsAutoDelete() { return m_autoDelete; }

		virtual size_t Read(void* p_data, size_t p_size = 0) = 0;
		virtual size_t Write(const void* p_data, size_t p_size) = 0;
		virtual void Clear() = 0;
		virtual void Close(bool p_deleteStream = false) = 0;

		virtual std::string GetLine(size_t p_dataSize, char delim = '\n') = 0;
		virtual int Peek() = 0;

		virtual bool End() = 0;
		virtual void Seek(size_t p_newPosition) = 0;
		virtual size_t Tell() = 0;

	protected:
		size_t m_size;
		std::string m_name;
		DataAccess m_dataAccess;
		bool m_autoDelete;
	};
}