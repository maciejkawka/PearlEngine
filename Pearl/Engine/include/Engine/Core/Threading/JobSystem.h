#pragma once

#include "JobDefines.h"
#include "JobWorker.h"

#include "Core/Utils/Singleton.h"

namespace PrCore::Threading {

	class JobSystem : public Utils::Singleton<JobSystem> {
	public:
		JobSystem(size_t p_workerNumber);
		~JobSystem();

		template<typename Func, typename... Args>
		JobStatePtr Schedule(std::string_view p_name, Func&& p_function, Args&&... p_args);

		// To add later
		//template<typename Func, typename... Args>
		//JobStatePtr ScheduleBatch(size_t p_batchSize, size_t p_jobNumber, std::string_view p_name, Func&& p_function, Args&&... p_args);

		void   WaitAll();

		void   PauseWorkers(bool p_pause);
		bool   GetWorkersPaused();
		size_t GetWorkerNum();

	private:
		std::vector<JobWorkerPtr> m_workers;
		std::atomic<size_t>       m_nextWorker;
		std::atomic<bool>         m_paused;

		std::atomic<size_t>       m_nextJobId;
	};
}

#include "Core/Threading/JobSystem.inl"