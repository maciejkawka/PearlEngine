#pragma once
#include"spdlog/spdlog.h"

#define PRLOG_DEBUG(...) PrCore::Utils::Logger::GetMainLogger()->debug(__VA_ARGS__);
#define PRLOG_INFO(...) PrCore::Utils::Logger::GetMainLogger()->info(__VA_ARGS__);
#define PRLOG_WARN(...) PrCore::Utils::Logger::GetMainLogger()->warn(__VA_ARGS__);
#define PRLOG_ERROR(...) PrCore::Utils::Logger::GetMainLogger()->error(__VA_ARGS__);

#define PRLOG_DEBUGF(...) PrCore::Utils::Logger::GetFileLogger()->debug(__VA_ARGS__);
#define PRLOG_INFOF(...) PrCore::Utils::Logger::GetFileLogger()->info(__VA_ARGS__);
#define PRLOG_WARNF(...) PrCore::Utils::Logger::GetFileLogger()->warn(__VA_ARGS__);
#define PRLOG_ERRORF(...) PrCore::Utils::Logger::GetFileLogger()->error(__VA_ARGS__);

namespace PrCore::Utils {

	class Logger {		
	
	public:
		Logger() = delete;
		~Logger() = delete;

		static void Init();

		inline static std::shared_ptr<spdlog::logger> GetMainLogger() { return s_mainLogger; }
		inline static std::shared_ptr<spdlog::logger> GetFileLogger() { return s_fileLogger; }

	private:
		static std::shared_ptr<spdlog::logger> s_mainLogger;
		static std::shared_ptr<spdlog::logger> s_fileLogger;
	};
}