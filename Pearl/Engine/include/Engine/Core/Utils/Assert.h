#pragma once

#include <string_view>

#ifdef PR_ASSERTENABLE
#define PR_ASSERT(EX, ...) (void)((EX) ||  (PrCore::Utils::PrAssertImpl(#EX, __FILE__, __LINE__, ## __VA_ARGS__),0))
#else
#define PR_ASSERT(EX, ...)
#endif

namespace PrCore::Utils {
	
	void PrAssertImpl(const char* p_msg, const char* p_file, int p_line, std::string_view p_info = "");
}

