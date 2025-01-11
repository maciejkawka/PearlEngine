#include "Core/Common/pearl_pch.h"

#include "Core/Threading/JobWorker.h"

#include <optional>

using namespace PrCore::Threading;

JobWorker::JobWorker(std::string_view p_name) :
	m_id(0),
	m_name(p_name)
{}

int JobWorker::ThreadLoop()
{
	m_id = ::GetCurrentThreadId();

	while (!ShouldTerminate())
	{
		// Wait if thread is paused
		if (ShouldPause())
		{
			std::unique_lock lock{ m_wakeLock };
			m_wakeCondition.wait(lock, [&]() {return !this->ShouldPause() || m_terminate.load(); });
		}

		bool jobProcessed = false;

		// Try to get job from the buffer
		if (!jobProcessed)
		{
			std::optional<JobDesc> jobDesc;
			{
				std::lock_guard lock{ m_jobBufferLock };
				if (m_jobBuffer.size() > 0)
				{
					jobDesc = m_jobBuffer.front();
					m_jobBuffer.pop_front();
				}
			}

			if (jobDesc)
			{
				ProcessJob(*jobDesc);
				jobProcessed = true;
			}
		}

		// Steal job from other workers
		if (!jobProcessed)
		{
			JobDesc jobDesc;
			// Iterate workers and steal the job
			for (auto& worker : m_stealWorkers)
			{
				if (worker->StealJob(jobDesc))
				{
#if JOB_SYSTEM_DEBUG_LOG
					PRLOG_INFO("Job stolen from {} <queue size: {}> by {} <queue size: {}> ", worker->m_name, worker->m_jobBuffer.size() + 1, m_name, m_jobBuffer.size());
#endif
					ProcessJob(jobDesc);
					jobProcessed = true;
					break;
				}
			}
		}

		// Go to sleep
		if (!jobProcessed)
		{
			// Notify that no more work to do and go sleep
			m_isBusy.store(false);
			m_idleCondition.notify_all();
	
			std::unique_lock lock{ m_wakeLock };;
			m_wakeCondition.wait(lock, [&]() { return !m_jobBuffer.empty() || m_terminate.load() || ShouldPause(); });
			m_isBusy.store(true);
		}
	}

	return 0;
}

void JobWorker::SetPaused(bool isPaused)
{
	m_pause.store(isPaused);
	Notify();
}

void JobWorker::Terminate()
{
	m_terminate.store(true);
	Notify();
}

void JobWorker::AddJobRequest(JobDesc&& p_jobDesc)
{
	PR_ASSERT(GetCurrentThreadId() != m_id, "Recursive job request! Not supported!");

	std::lock_guard lock{ m_jobBufferLock };
	m_jobBuffer.push_back(std::move(p_jobDesc));
}

bool JobWorker::StealJob(JobDesc& p_jobDesc)
{
	// Steal only if worker is busy
	if (!IsBusy())
		return false;

	std::lock_guard lock{ m_jobBufferLock };
	if(m_jobBuffer.size() == 0)
		return false;

	p_jobDesc = m_jobBuffer.back();
	m_jobBuffer.pop_back();
	return true;
}

void JobWorker::Notify()
{
	m_wakeCondition.notify_one();
}

bool JobWorker::IsBusy()
{
	return m_isBusy.load();
}

void JobWorker::WaitForIdle()
{
	std::unique_lock lock{ m_idleLock };
	m_idleCondition.wait(lock, [&]() {return !IsBusy(); });
}

void JobWorker::SetStealWorkers(const std::vector<std::shared_ptr<JobWorker>>& p_workers)
{
	m_stealWorkers = p_workers;
	
	m_stealWorkers.erase(std::remove_if(m_stealWorkers.begin(), m_stealWorkers.end(),
		[&](JobWorkerPtr p_worker) { return p_worker->m_id == this->m_id; }), m_stealWorkers.end());
}

void JobWorker::ProcessJob(JobDesc& p_jobDesc)
{
#if JOB_SYSTEM_DEBUG_LOG
	PRLOG_INFO("JobWorker \"{}\" starting job: \"{}\"", m_name, p_jobDesc.name);
#endif

	p_jobDesc.state->m_isDone.store(false);
	p_jobDesc.functionPtr();
	p_jobDesc.state->m_isDone.store(true);
	p_jobDesc.state->m_finishedCondition.notify_all();
}

bool JobWorker::ShouldTerminate()
{
	std::lock_guard lock{ m_jobBufferLock };
	return m_terminate.load() && m_jobBuffer.empty();
}
