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
	consoleSink->set_pattern("[%T %n]\n%^%l: %v\n%$");

	s_mainLogger = std::make_shared<spdlog::logger>("Pearl", consoleSink);
	s_mainLogger->flush_on(spdlog::level::debug);
	s_mainLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(s_mainLogger);

	auto time = std::time(NULL);
	std::tm* now = std::localtime(&time);
	auto fileName = std::to_string(now->tm_year + 1900) + "-"
		+ std::to_string(now->tm_mon) + "-"
		+ std::to_string(now->tm_mday) + "_"
		+ std::to_string(now->tm_hour) + "-"
		+ std::to_string(now->tm_min) + "_"
		+ "PearlLog.log";
	auto directory = "Logging/";

	//File logger
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(directory + fileName, true);
	fileSink->set_pattern("[%T %n Thread ID: %t] %l: %v");
	s_fileLogger = std::make_shared<spdlog::logger>("Pearl_File", fileSink);
	s_fileLogger->flush_on(spdlog::level::debug);
	s_fileLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(s_fileLogger);

	PRLOG_INFO("Init Logger");
}
