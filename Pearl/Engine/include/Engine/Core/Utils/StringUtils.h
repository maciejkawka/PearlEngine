#pragma once 
#include<string>

namespace PrCore::Utils {

	class StringUtils {
	public:
		StringUtils() = delete;
		StringUtils(StringUtils& p_copy) = delete;

		static void ResizeToFitContains(std::string& p_text);
	};
}