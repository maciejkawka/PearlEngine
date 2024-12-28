#pragma once
#include<string>

#include "Core/Utils/JSONParser.h"
#include"JSON/json.hpp"

using json = nlohmann::json;

namespace PrCore::File {

	class ConfigFile {
	public:
		ConfigFile() = default;
		~ConfigFile() = default;

		bool SaveToFile(std::string_view p_filePath);
		bool OpenFromFile(std::string_view p_filePath);

		std::string_view GetFilePath() { return m_filePath; }

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
		json        m_jsonFile;
		std::string m_filePath;
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