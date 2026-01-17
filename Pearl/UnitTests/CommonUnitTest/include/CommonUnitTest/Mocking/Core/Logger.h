#pragma once

#include "CommonUnitTest/Common/common.h"
#include "Core/Utils/ILogger.h"

#include <string>

namespace Mock {

	class MockLogger : public PrCore::Utils::ILogger {
		void Log(std::string_view msg) override {}
		void Warning(std::string_view msg) override {}
		void Error(std::string_view msg) override { GTEST_FATAL_FAILURE_(std::string{ msg }.data()); }
	};
}