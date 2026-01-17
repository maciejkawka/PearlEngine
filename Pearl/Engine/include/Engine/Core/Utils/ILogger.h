#pragma once

#include "SystemProvider.h"

// Include that to have FMT lib I will add that as dependency later
#include "spdlog/spdlog.h"

#include <string_view>
#include <memory>

#define PRLOG_INFO(...)  PrSystems::Get<PrCore::Utils::ILogger>()->Log(fmt::format(__VA_ARGS__));
#define PRLOG_WARN(...)  PrSystems::Get<PrCore::Utils::ILogger>()->Warning(fmt::format(__VA_ARGS__));
#define PRLOG_ERROR(...) PrSystems::Get<PrCore::Utils::ILogger>()->Error(fmt::format(__VA_ARGS__));

namespace PrCore::Utils {

	class ILogger : public ISystem {
	public:
		virtual ~ILogger() = default;

		virtual void Log(std::string_view msg) = 0;;
		virtual void Warning(std::string_view msg) = 0;
		virtual void Error(std::string_view msg) = 0;
	};
}