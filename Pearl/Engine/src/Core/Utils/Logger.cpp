#include"Core/Common/pearl_pch.h"

#include "Core/Utils/Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace PrCore::Utils;

std::shared_ptr<spdlog::logger> Logger::s_mainLogger = nullptr;
std::shared_ptr<spdlog::logger> Logger::s_fileLogger = nullptr;

void Logger::Init()
{
	//Main logger
	auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
	consoleSink->set_pattern("<%T> %v%$");

	s_mainLogger = std::make_shared<spdlog::logger>("Pearl", consoleSink);
	s_mainLogger->flush_on(spdlog::level::debug);
	s_mainLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(s_mainLogger);

	//File logger
	auto fileName = "Logging/PearlLog.log";
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true);
	fileSink->set_pattern("<%T> %v%$");
	s_fileLogger = std::make_shared<spdlog::logger>("Pearl_File", fileSink);
	s_fileLogger->flush_on(spdlog::level::debug);
	s_fileLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(s_fileLogger);

	PRLOG_INFO("Init Logger");
}
