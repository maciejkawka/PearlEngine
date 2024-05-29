#include"Core/Common/pearl_pch.h"

#include"Core/Utils/StringUtils.h"

namespace PrCore::StringUtils {

	void ResizeToFitContains(std::string& p_text)
	{
		auto stringEnd = p_text.find_first_of('\0', 0);
		if (stringEnd == std::string::npos)
			return;

		p_text.resize(stringEnd);
		p_text.shrink_to_fit();
	}
}