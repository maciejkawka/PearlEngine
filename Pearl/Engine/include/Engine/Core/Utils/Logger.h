#pragma once

#include "Core/Utils/ILogger.h"

namespace PrCore::Utils {

	class Logger final : public ILogger {
	public:
		Logger();
		virtual ~Logger();

		void Log(std::string_view msg) override;
		void Warning(std::string_view msg) override;
		void Error(std::string_view msg) override;

	private:
		std::shared_ptr<spdlog::logger> m_mainLogger;
		std::shared_ptr<spdlog::logger> m_fileLogger;
	};
}