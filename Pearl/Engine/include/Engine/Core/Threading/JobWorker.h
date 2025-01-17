#pragma once

#include "IThread.h"
#include "JobDefines.h"

#include <optional>

namespace PrCore::Threading {

	class JobWorker : public IThread {
	public:
		JobWorker(std::string_view p_name);
		~JobWorker();

		int ThreadLoop() override;

		void                    AddJobRequest(JobDesc&& p_jobDesc);
		std::optional<JobDesc>  StealJob();

		bool IsBusy();
		void WaitForIdle();

		void SetStealWorkers(const std::vector<std::shared_ptr<JobWorker>>& p_workers);

	protected:
		bool ShouldPause() override { return m_pause.load(); }
		bool ShouldTerminate() override;

		void SetPaused(bool isPaused) override;
		void Terminate() override;

		bool IsPaused() override { return m_pause; }
		bool IsTerminated() override { return m_terminate; }

		void ProcessJob(const JobDesc& p_jobDesc);

		std::string       m_name;
		size_t            m_id;

		std::deque<JobDesc> m_jobBuffer;

		std::condition_variable  m_idleCondition;
		std::mutex               m_idleLock;

		std::condition_variable  m_wakeCondition;
		std::mutex               m_workerLock;

		std::vector<std::weak_ptr<JobWorker>> m_stealWorkers;

		std::atomic<bool>       m_isBusy;
		std::atomic<bool>       m_pause;
		std::atomic<bool>       m_terminate;
	};
	using JobWorkerPtr = std::shared_ptr<JobWorker>;
}