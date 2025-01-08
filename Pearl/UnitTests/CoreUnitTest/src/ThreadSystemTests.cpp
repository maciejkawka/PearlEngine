#include <CommonUnitTest/Common/common.h>

#include "Core/Threading/ThreadSystem.h"
#include "Core/Utils/Logger.h"
#include "Core/Utils/StringUtils.h"

#include <future>

using namespace PrCore::Threading;

class TestThread : public IThread
{
public:
	int ThreadLoop() override
	{
		while (!ShouldTerminate())
		{
		}

		return 0;
	}

protected:
	bool ShouldPause() override
	{
		return m_shouldPause;
	}

	bool ShouldTerminate() override
	{
		return m_shouldTerniate;
	}

	void SetPaused(bool isPaused) override
	{
		m_shouldPause = isPaused;
	}

	void Terminate() override
	{
		m_shouldTerniate = true;
	}

	bool IsPaused() override
	{
		return m_shouldPause;
	}

	bool IsTerminated() override
	{
		return m_shouldTerniate;
	}

	std::atomic<bool> m_shouldPause;
	std::atomic<bool> m_shouldTerniate;
};

class ThreadingSystemTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Utils::Logger::Init();
		PrCore::Threading::ThreadSystem::Init();
	}

	static void TearDownTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Threading::ThreadSystem::Terminate();
		PrCore::Utils::Logger::Terminate();
	}
};

TEST_F(ThreadingSystemTest, SpawnThread)
{
	auto threadingSystem = PrCore::Threading::ThreadSystem::GetInstancePtr();

	auto thread = std::make_shared<TestThread>();
	ThreadConfig config;
	config.name = "TestThread";
	config.affinityMask = 1;

	EXPECT_TRUE(threadingSystem->SpawnThread(thread, config));
	EXPECT_FALSE(threadingSystem->IsThreadPaused(thread));
	EXPECT_FALSE(threadingSystem->IsThreadTerminated(thread));

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(200ms);

	threadingSystem->PauseThread(thread, true);
	EXPECT_TRUE(threadingSystem->IsThreadPaused(thread));
	EXPECT_FALSE(threadingSystem->IsThreadTerminated(thread));

	threadingSystem->PauseThread(thread, false);
	EXPECT_FALSE(threadingSystem->IsThreadPaused(thread));
	EXPECT_FALSE(threadingSystem->IsThreadTerminated(thread));

	threadingSystem->TerminateThread(thread);
	EXPECT_FALSE(threadingSystem->IsThreadPaused(thread));
	EXPECT_TRUE(threadingSystem->IsThreadTerminated(thread));

	threadingSystem->JoinThread(thread);
	EXPECT_EQ(threadingSystem->GetThreadNum(), 0);
}

TEST_F(ThreadingSystemTest, JoinThread)
{
	auto threadingSystem = PrCore::Threading::ThreadSystem::GetInstancePtr();

	auto thread = std::make_shared<TestThread>();
	ThreadConfig config;
	config.name = "TestThread";
	config.affinityMask = 1;

	EXPECT_TRUE(threadingSystem->SpawnThread(thread, config));
	
	auto future = std::async([&]() {
		threadingSystem->JoinThread(thread);
		return true;
		});

	auto future1 = std::async([&]() {
		threadingSystem->JoinThread(thread);
		return true;
		});

	auto future2 = std::async([&]() {
		threadingSystem->JoinThread(thread);
		return true;
		});

	auto future3 = std::async([&]() {
		threadingSystem->JoinThread(thread);
		return true;
		});

	auto future4 = std::async([&]() {
		threadingSystem->JoinThread(thread);
		return true;
		});

	EXPECT_EQ(threadingSystem->GetThreadNum(), 1);

	using namespace std::chrono_literals;
	std::this_thread::sleep_for(200ms);
	threadingSystem->TerminateThread(thread);

	EXPECT_TRUE(future.get());
	EXPECT_TRUE(future1.get());
	EXPECT_TRUE(future2.get());
	EXPECT_TRUE(future3.get());
	EXPECT_TRUE(future4.get());

	EXPECT_EQ(threadingSystem->GetThreadNum(), 0);
}

TEST_F(ThreadingSystemTest, ThreadInfo)
{
	const char* threadName = "TestThread";
	auto threadingSystem = PrCore::Threading::ThreadSystem::GetInstancePtr();

	auto thread = std::make_shared<TestThread>();
	ThreadConfig config;
	config.name = threadName;
	config.affinityMask = 8;

	EXPECT_TRUE(threadingSystem->SpawnThread(thread, config));

	auto threadInfo = threadingSystem->GetThreadInfo(thread);
	EXPECT_EQ(threadInfo.affinityMask, 8);
	EXPECT_STREQ(threadInfo.name.c_str(), threadName);
	EXPECT_EQ(threadInfo.stackSize, 0);

	threadingSystem->TerminateThread(thread);
	threadingSystem->JoinThread(thread);
	EXPECT_EQ(threadingSystem->GetThreadNum(), 0);
}

TEST_F(ThreadingSystemTest, StressTest)
{
	using namespace std::chrono_literals;

	const int threadNumber = 20;
	const auto waitTime = 500ms;

	auto threadingSystem = PrCore::Threading::ThreadSystem::GetInstancePtr();
	// Aggressively spawn and terminate threads, this is to spot issues with thread registry and data racing 
	for(int i=0;i<10;i++)
	{
		std::vector<IThreadPtr> threads;

		for (int i = 0; i < 20; i++)
		{
			auto thread = std::make_shared<TestThread>();
			ThreadConfig config;
			config.name = PrCore::StringUtils::ToString(i);
			EXPECT_TRUE(threadingSystem->SpawnThread(thread, config));

			threads.push_back(thread);
		}

		EXPECT_EQ(threadingSystem->GetThreadNum(), 20);

		for (int i = 0; i < threadNumber; i++)
			threadingSystem->PauseThread(threads[i], true);

		for (int i = 0; i < threadNumber; i++)
			EXPECT_TRUE(threadingSystem->IsThreadPaused(threads[i]));

		for (int i = 0; i < threadNumber; i++)
			threadingSystem->PauseThread(threads[i], false);

		for (int i = 0; i < threadNumber; i++)
			EXPECT_FALSE(threadingSystem->IsThreadPaused(threads[i]));
		
		std::this_thread::sleep_for(waitTime);

		for (int i = 0; i < threadNumber; i++)
			threadingSystem->TerminateThread(threads[i]);

		for (int i = 0; i < threadNumber; i++)
			threadingSystem->JoinThread(threads[i]);

		EXPECT_EQ(threadingSystem->GetThreadNum(), 0);
	}

	EXPECT_EQ(threadingSystem->GetThreadNum(), 0);
}
