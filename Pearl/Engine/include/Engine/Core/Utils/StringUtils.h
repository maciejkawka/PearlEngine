#pragma once

#include<string>
#include<string_view>
#include<charconv>

namespace PrCore::StringUtils {

	void ResizeToFitContains(std::string& p_text);

	inline void ToLower(char* p_text)
	{
		char* ptr = p_text;
		while(*ptr != '\0')
		{
			*ptr = static_cast<char>(std::tolower(static_cast<unsigned int>(*ptr)));
			ptr++;
		}
	}

	inline std::string ToLower(const std::string& p_text)
	{
		std::string retPath;
		retPath.resize(p_text.size());
		for (int i=0;i< p_text.size(); i++)
		{
			retPath[i] = static_cast<char>(std::tolower(static_cast<unsigned int>(p_text[i])));
		}

		return retPath;
	}

	inline void ToLower(std::string& p_text)
	{
		for(auto& ch : p_text)
		{
			ch = static_cast<char>(std::tolower(static_cast<unsigned int>(ch)));
		}
	}

	inline void ToUpper(char* p_text)
	{
		char* ptr = p_text;
		while (*ptr != '\0')
		{
			*ptr = static_cast<char>(std::toupper(static_cast<unsigned int>(*ptr)));
			ptr++;
		}
	}

	inline void ToUpper(std::string& p_text)
	{
		for (auto& ch : p_text)
		{
			ch = static_cast<char>(std::toupper(static_cast<unsigned int>(ch)));
		}
	}

	inline std::string ToUpper(const std::string& p_text)
	{
		std::string retPath;
		retPath.resize(p_text.size());
		for (int i = 0; i < p_text.size(); i++)
		{
			retPath[i] = static_cast<char>(std::toupper(static_cast<unsigned int>(p_text[i])));
		}

		return retPath;
	}

	template<typename T>
	std::string ToString(const T& p_value)
	{
		return std::to_string(p_value);
	}

	template<typename T>
	bool FromString(std::string_view p_view, T& p_value)
	{
		return std::from_chars(p_view.data(), p_view.data() + p_view.size(), p_value).ec == std::errc{};
	}

	template<typename T>
	bool FromString(std::string_view p_view, T& p_value, const size_t p_pos)
	{
		return std::from_chars(p_view.data(), p_view.data() + p_pos, p_value).ec == std::errc{};
	}
}