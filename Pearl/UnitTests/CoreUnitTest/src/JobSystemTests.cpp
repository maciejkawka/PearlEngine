#include <CommonUnitTest/Common/common.h>

#include "Core/Threading/ThreadSystem.h"
#include "Core/Threading/JobSystem.h"
#include "Core/Utils/Logger.h"
#include "Core/Utils/StringUtils.h"

#include <future>

using namespace PrCore::Threading;

class JobSystemTest : public ::testing::Test {
public:
	static void SetUpTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Utils::Logger::Init();
		PrCore::Threading::ThreadSystem::Init();
		PrCore::Threading::JobSystem::Init(8);

		auto workerNum = PrCore::Threading::JobSystem::GetInstance().GetWorkerNum();
		EXPECT_EQ(workerNum, 8);
	}

	static void TearDownTestSuite()
	{
		//This will be replaced with mocked versions in the future when I implement system localizer 
		PrCore::Threading::JobSystem::Terminate();
		PrCore::Threading::ThreadSystem::Terminate();
		PrCore::Utils::Logger::Terminate();
	}
};

int JobValue = 0;
void JobFunction(int p_val)
{
	JobValue = p_val;
}

class JobObject
{
public:
	int value = 0;
	void MemberFunction(int p_val)
	{
		value = p_val;
	}
};

TEST_F(JobSystemTest, ScheduleJob)
{
	auto jobPtr = JobSystem::GetInstancePtr();

	// Free function
	auto state = jobPtr->Schedule("FreeFunction", &JobFunction, 50);
	EXPECT_FALSE(state->IsDone());
	state->Wait();
	EXPECT_TRUE(state->IsDone());
	EXPECT_EQ(JobValue, 50);

	// Member Function
	{
		JobObject object;
		object.value = 10;
		auto state = jobPtr->Schedule("MemberFunction", &JobObject::MemberFunction, &object, 80);
		EXPECT_FALSE(state->IsDone());
		state->Wait();
		EXPECT_TRUE(state->IsDone());
		EXPECT_EQ(object.value, 80);
	}

	// Lambda Function
	{
		int value = 0;
		auto state = jobPtr->Schedule("LambdaFunction", [&]() {
			value = 100;
			});
		EXPECT_FALSE(state->IsDone());
		state->Wait();
		EXPECT_TRUE(state->IsDone());
		EXPECT_EQ(value, 100);
	}
}

TEST_F(JobSystemTest, PauseWaitAllJobs)
{
	auto jobPtr = JobSystem::GetInstancePtr();
	jobPtr->PauseWorkers(true);

	std::atomic<int> value = 0;
	for(int i=0;i<100;i++)
	{
		auto state = jobPtr->Schedule("LambdaFunction", [&]() {
			using namespace std::chrono_literals;
			std::this_thread::sleep_for(20ms);
			value++;
			});
	}

	EXPECT_EQ(value, 0);

	jobPtr->WaitAll();
	jobPtr->PauseWorkers(false);
	jobPtr->WaitAll();
	EXPECT_FALSE(jobPtr->GetWorkersPaused());
	EXPECT_EQ(value, 100);
}

TEST_F(JobSystemTest, StressTest)
{
	auto jobPtr = JobSystem::GetInstancePtr();

	std::atomic<int> value = 0;
	auto lambda = [&]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20ms);
		value++;
	};

	for (int i = 0; i < 100; i++)
		jobPtr->Schedule("LambdaFunction", lambda);

	std::async([&]() {
		for (int i = 0; i < 100; i++)
			jobPtr->Schedule("LambdaFunction", lambda);
	});

	std::async([&]() {
		for (int i = 0; i < 100; i++)
			jobPtr->Schedule("LambdaFunction", lambda);
		});

	std::async([&]() {
		for (int i = 0; i < 100; i++)
			jobPtr->Schedule("LambdaFunction", lambda);
		});

	jobPtr->WaitAll();
	EXPECT_EQ(value, 400);
}

TEST_F(JobSystemTest, JobStealing)
{
	auto jobPtr = JobSystem::GetInstancePtr();

	std::atomic<int> value = 0;

	auto longLambda = [&]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(100ms);
		value++;
	};

	auto shortLambda = [&]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(10ms);
		value++;
	};

	std::async([&]() {
	for (int i = 0; i < 100; i++)
	{
		jobPtr->Schedule("longLambda", longLambda);
		jobPtr->Schedule("shortLambda", shortLambda);
	}});

	std::async([&]() {
		for (int i = 0; i < 100; i++)
		{
			jobPtr->Schedule("longLambda", longLambda);
			jobPtr->Schedule("shortLambda", shortLambda);
	}});

	jobPtr->WaitAll();
	EXPECT_EQ(value, 400);
}

TEST_F(JobSystemTest, TerminatePaused)
{
	auto jobPtr = JobSystem::GetInstancePtr();

	std::atomic<int> value = 0;
	auto lambda = [&]() {
		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20ms);
		value++;
	};

	jobPtr->PauseWorkers(true);
	jobPtr->Schedule("lambda",lambda);
	jobPtr->Schedule("lambda",lambda);
	jobPtr->Schedule("lambda",lambda);
	jobPtr->Schedule("lambda",lambda);

	EXPECT_EQ(value, 0);
	PrCore::Threading::JobSystem::Terminate();
	EXPECT_EQ(value, 4);

	PrCore::Threading::JobSystem::Init(8);
}
