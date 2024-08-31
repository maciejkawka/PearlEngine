#pragma once
#include<string>

#include "Core/Utils/JSONParser.h"
#include"JSON/json.hpp"

using json = nlohmann::json;

namespace PrCore::Filesystem {

	class ConfigFile {
	public:
		ConfigFile(const std::string& p_fileName);
		ConfigFile() :
			m_isValid(false)
		{}

		~ConfigFile() {}

		bool Open(const std::string& p_fileName);
		void Create(const std::string& p_fileName);
		void Override();

		inline bool IsValid() const { return m_isValid; }

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

	template<>
	inline void ConfigFile::GetSetting<PrCore::Math::vec2>(const std::string& p_settingName, PrCore::Math::vec2& p_value)
	{
		if (m_jsonFile.empty())
			return;

		p_value = PrCore::Utils::JSONParser::ToVec2(m_jsonFile[p_settingName]);
	}

	template<>
	inline void ConfigFile::GetSetting<PrCore::Math::vec4>(const std::string& p_settingName, PrCore::Math::vec4& p_value)
	{
		if (m_jsonFile.empty())
			return;

		p_value = PrCore::Utils::JSONParser::ToVec4(m_jsonFile[p_settingName]);
	}

	template<>
	inline void ConfigFile::GetSetting<PrCore::Math::vec3>(const std::string& p_settingName, PrCore::Math::vec3& p_value)
	{
		if (m_jsonFile.empty())
			return;

		p_value = PrCore::Utils::JSONParser::ToVec3(m_jsonFile[p_settingName]);
	}

	template<>
	inline void ConfigFile::GetSetting<PrCore::Math::mat4>(const std::string& p_settingName, PrCore::Math::mat4& p_value)
	{
		if (m_jsonFile.empty())
			return;

		p_value = PrCore::Utils::JSONParser::ToMat4(m_jsonFile[p_settingName]);
	}

	template<>
	inline void ConfigFile::GetSetting<PrCore::Math::mat3>(const std::string& p_settingName, PrCore::Math::mat3& p_value)
	{
		if (m_jsonFile.empty())
			return;

		p_value = PrCore::Utils::JSONParser::ToMat3(m_jsonFile[p_settingName]);
	}

#define GET_CONFIG_SETTING_NAME(configVar, settingsName) \
		GetSetting(#settingsName, configVar.settingsName)
}