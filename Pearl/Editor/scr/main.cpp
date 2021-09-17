#pragma once
#include<iostream>
#include"Core/Utils/Logger.h"
#include<future>
#include<Windows.h>

int main()
{
	PrCore::Utils::Logger::Init();

	std::string e = "ELO";
	PRLOG_DEBUG("Test debug {0}", 90);
	PRLOG_DEBUGF("Test debug {0}", 90);

	std::thread t1([]() {
		PRLOG_INFOF("TEST1");
		Sleep(1000);
		});
	std::thread t2([]() {
		PRLOG_INFOF("TEST2");
		Sleep(1000);
		});
	std::thread t3([]() {
		PRLOG_INFOF("TEST3");
		Sleep(1000);
		});


	PRLOG_DEBUGF("Test debug {0}", 90);

	t1.join();
	t2.join();
	t3.join();

	PRLOG_INFO("ELO");
	PRLOG_WARN("Test Warning");
	PRLOG_ERROR("Test Error");

	return 0;
}
