#include "Core/Common/pearl_pch.h"

#include "Core/Threading/JobWorker.h"

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
			std::unique_lock lock{ m_workerLock };
			m_isBusy.store(false);
			m_wakeCondition.wait(lock, [&]() {return !m_pause.load() || m_terminate.load(); });
			m_isBusy.store(true);
		}

		// Try to get job from the buffer
		bool jobProcessed = false;
		if (!jobProcessed)
		{
			std::optional<JobDesc> jobDesc;
			{
				std::lock_guard lock{ m_workerLock };
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
			// Iterate workers and steal the job
			for (auto& worker : m_stealWorkers)
			{
				auto sharedPtr = worker.lock();
				if (sharedPtr)
				{
					auto jobDesc = sharedPtr->StealJob();
					if (jobDesc)
					{
#if JOB_SYSTEM_DEBUG_LOG
						PRLOG_INFO("Job stolen from {} <queue size: {}> by {} <queue size: {}> ", worker->m_name, worker->m_jobBuffer.size() + 1, m_name, m_jobBuffer.size());
#endif
						ProcessJob(*jobDesc);
						jobProcessed = true;
						break;
					}
				}
			}
		}

		// No job was processed and jobBuffer is empty
		std::unique_lock lock{ m_workerLock };
		if (!jobProcessed && m_jobBuffer.empty())
		{
			m_isBusy.store(false);

			// Notify that no more work to do and go sleep
			m_idleCondition.notify_all();
		
			m_wakeCondition.wait(lock, [&]() {
				return !m_jobBuffer.empty() || m_terminate.load() || m_pause.load();
				});

			m_isBusy.store(true);
		}
	}

	return 0;
}

void JobWorker::SetPaused(bool isPaused)
{
	m_pause.store(isPaused);
	m_wakeCondition.notify_one();
}

void JobWorker::Terminate()
{
	m_terminate.store(true);
	m_wakeCondition.notify_one();
}

void JobWorker::AddJobRequest(JobDesc&& p_jobDesc)
{
	PR_ASSERT(GetCurrentThreadId() != m_id, "Recursive job request! Not supported!");

	{
		std::lock_guard lock{ m_workerLock };
		m_jobBuffer.push_back(std::move(p_jobDesc));
	}

	m_wakeCondition.notify_one();
}

std::optional<JobDesc> JobWorker::StealJob()
{
	// Steal only if worker is busy
	if (!IsBusy())
		return std::nullopt;

	std::lock_guard lock{ m_workerLock };
	if (m_jobBuffer.empty())
		return std::nullopt;

	auto jobDesc = std::move(m_jobBuffer.back());
	m_jobBuffer.pop_back();
	return jobDesc;
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
	for (auto& worker : p_workers)
	{
		if (worker->m_id != m_id)
			m_stealWorkers.push_back(worker);
	}
}

void JobWorker::ProcessJob(const JobDesc& p_jobDesc)
{
#if JOB_SYSTEM_DEBUG_LOG
	PRLOG_INFO("JobWorker \"{}\" starting job: \"{}\"", m_name, p_jobDesc.name);
#endif

	auto jobState = p_jobDesc.state;
	{
		std::lock_guard lock{ jobState->m_finishedLock };
		p_jobDesc.functionPtr();
		jobState->m_isDone.store(true);
	}

	jobState->m_finishedCondition.notify_all();
}

bool JobWorker::ShouldTerminate()
{
	std::lock_guard lock{ m_workerLock };
	return m_terminate.load() && m_jobBuffer.empty();
}

JobWorker::~JobWorker()
{
	PR_ASSERT(m_jobBuffer.empty(), "Job buffer must be emtry!");
	m_stealWorkers.clear();
}
