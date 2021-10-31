#include<string>
#include"JSON/json.hpp"

using json = nlohmann::json;

namespace PrCore::Filesystem {

	class ConfigFile {
	public:
		ConfigFile(const std::string& p_fileName);
		ConfigFile() {}

		~ConfigFile() {}

		void Open(const std::string& p_fileName);
		void Create(const std::string& p_fileName);
		void Override();

		template<typename T>
		void SetSetting(const std::string& p_settingName, T settingValue)
		{
			m_jsonFile[p_settingName] = settingValue;
		}

		template<typename T = int>
		T GetSetting(const std::string& p_settingName)
		{
			auto setting = m_jsonFile[p_settingName];
			return setting;
		}

		template<typename T>
		void GetSetting(const std::string& p_settingName, T& p_value)
		{
			p_value = m_jsonFile[p_settingName];
		}

		//Can return only int 
		int operator[](const std::string& p_settingName)
		{
			return GetSetting<int>(p_settingName);
		}

	private:
		json m_jsonFile;
		std::string m_fileName;
	};
}