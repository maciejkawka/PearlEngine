#include<string>
#include"JSON/json.hpp"

using json = nlohmann::json;

namespace PrCore::Filesystem {

	class ConfigFile {
	public:
		ConfigFile(const std::string& p_fileName);
		ConfigFile():
		m_isValid(false)
		 {}

		~ConfigFile() {}

		bool Open(const std::string& p_fileName);
		void Create(const std::string& p_fileName);
		void Override();

		inline bool isValid() { return m_isValid; }

		template<typename T>
		void SetSetting(const std::string& p_settingName, T settingValue)
		{
			if (m_jsonFile.empty())
				return;

			m_jsonFile[p_settingName] = settingValue;
		}

		template<typename T = int>
		T GetSetting(const std::string& p_settingName)
		{
			if (m_jsonFile.empty())
				return 0;
			auto setting = m_jsonFile[p_settingName];
			return setting;
		}

		template<typename T>
		void GetSetting(const std::string& p_settingName, T& p_value)
		{
			if (m_jsonFile.empty())
				return;

			p_value = m_jsonFile[p_settingName];
		}

		//Can return only int 
		int operator[](const std::string& p_settingName)
		{
			if (m_jsonFile.empty())
				return 0;

			return GetSetting<int>(p_settingName);
		}

	private:
		json m_jsonFile;
		std::string m_fileName;
		bool m_isValid;
	};
}