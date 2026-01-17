#include "Core/Common/pearl_pch.h"

#include "Core/Utils/Assert.h"
#include "Core/Utils/Logger.h"

namespace PrCore::Utils {

	void PrAssertImpl(const char* p_msg, const char* p_file, int p_line, std::string_view p_info)
	{
		PRLOG_ERROR("ASSERTION FILED! \nFILE: {}\nLINE: {}\nASSERT CAUSE: {}\nINFO: {}", p_file, p_line, p_msg, p_info);
		__debugbreak();
	}
}