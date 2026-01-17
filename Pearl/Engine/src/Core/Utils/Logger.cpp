#include"Core/Common/pearl_pch.h"

#include "Core/Utils/Logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

using namespace PrCore::Utils;

Logger::Logger()
{
	//Main logger
	auto consoleSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
	consoleSink->set_pattern("<%T> %v%$");

	m_mainLogger = std::make_shared<spdlog::logger>("Pearl", consoleSink);
	m_mainLogger->flush_on(spdlog::level::debug);
	m_mainLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(m_mainLogger);

	//File logger
	auto fileName = "Logging/PearlLog.log";
	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true);
	fileSink->set_pattern("<%T> %v%$");
	m_fileLogger = std::make_shared<spdlog::logger>("Pearl_File", fileSink);
	m_fileLogger->flush_on(spdlog::level::debug);
	m_fileLogger->set_level(spdlog::level::debug);
	spdlog::register_logger(m_fileLogger);

	Log("Init Logger");
}

Logger::~Logger()
{
	m_mainLogger.reset();
	m_fileLogger.reset();

	spdlog::shutdown();
}

void Logger::Log(std::string_view msg)
{
	m_mainLogger->info(msg);
	m_fileLogger->info(msg);
}

void Logger::Warning(std::string_view msg)
{
	m_mainLogger->warn(msg);
	m_fileLogger->warn(msg);
}

void Logger::Error(std::string_view msg)
{
	m_mainLogger->error(msg);
	m_fileLogger->error(msg);
}
