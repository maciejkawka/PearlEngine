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

	class BatchJobState {
	public:
		BatchJobState()
		{
			m_stateVector.reserve(64);
		}

		void Wait()
		{
			for (auto& state : m_stateVector)
				state->Wait();
		}

		bool IsDone()
		{
			bool done = false;
			for (auto& state : m_stateVector)
				done &= state->IsDone();

			return done;
		}

		void Append(JobStatePtr&& p_statePtr)
		{
			m_stateVector.push_back(std::move(p_statePtr));
		}

		BatchJobState& operator+=(JobStatePtr&& p_statePtr)
		{
			m_stateVector.push_back(std::move(p_statePtr));
			return *this;
		}

	private:
		std::vector<JobStatePtr> m_stateVector;
	};

	struct JobDesc
	{
		JobPtr      functionPtr;
		JobStatePtr state;
		size_t      id;
		std::string name;
	};
}