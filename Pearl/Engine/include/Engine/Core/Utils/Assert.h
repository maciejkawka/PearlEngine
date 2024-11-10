#pragma once
#include"Core/Utils/Logger.h"

namespace PrCore::Utils {
	
	template<typename ...Args>
	inline void pearl_assert(const char* p_msg, const char* p_file, int p_line, std::string p_info = "NONE");


#ifdef PR_ASSERTENABLE
	#define PR_ASSERT(EX, ...) (void)((EX) ||  (PrCore::Utils::pearl_assert(#EX, __FILE__, __LINE__, ## __VA_ARGS__),0))
#else
	#define PR_ASSERT(EX, ...)
#endif


}

template<typename ...Args>
inline void PrCore::Utils::pearl_assert(const char* p_msg, const char* p_file, int p_line, std::string p_info)
{
	PRLOG_ERROR("ASSERTION FILED! \nFILE: {0}\nLINE: {1}\nASSERT CAUSE: {2}\nINFO: {3}", p_file, p_line, p_msg, p_info);
	__debugbreak();
}