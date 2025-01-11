#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <queue>

#define JOB_SYSTEM_DEBUG_LOG 0

namespace PrCore::Threading {
	class JobWorker;

	class JobState {
	public:
		JobState() = default;

		void Wait();
		bool IsDone();

	private:
		std::atomic<bool>       m_isDone = false;
		std::condition_variable m_finishedCondition;
		std::mutex              m_finishedLock;

		friend JobWorker;
	};
	using JobStatePtr = std::shared_ptr<JobState>;
	using JobPtr = std::function<void()>;

	struct JobDesc
	{
		JobPtr      functionPtr;
		JobStatePtr state;
		size_t      id;
		std::string name;
	};
}