#include "Core/Common/pearl_pch.h"

#include "Core/Threading/JobSystem.h"
#include "Core/Threading/ThreadSystem.h"

using namespace PrCore::Threading;

JobSystem::JobSystem(size_t p_workerNumber)
{
	PR_ASSERT(p_workerNumber > 0, "Worker number is less than 1");

	auto threadSystem = ThreadSystem::GetInstancePtr();
	for (int i = 0; i < p_workerNumber; i++)
	{
		ThreadConfig config;
		config.name = "JobWorker_" + StringUtils::ToString(i);
		auto worker = std::make_shared<JobWorker>(config.name);

		threadSystem->SpawnThread(worker, config);
		m_workers.push_back(std::move(worker));
	}

	// Share workers vector to steal jobs from
	for (auto& worker : m_workers)
		worker->SetStealWorkers(m_workers);
	
	m_activeWorker.store(0);
	m_nextJobId.store(0);
	m_paused.store(false);
}

JobSystem::~JobSystem()
{
	auto threadSystem = ThreadSystem::GetInstancePtr();
	for (int i = 0; i < m_workers.size(); i++)
	{
		threadSystem->TerminateThread(m_workers[i]);
		threadSystem->JoinThread(m_workers[i]);
	}

	m_workers.clear();
}

void JobSystem::PauseWorkers(bool p_pause)
{
	auto threadSystem = ThreadSystem::GetInstancePtr();
	for (auto& worker : m_workers)
	{
		threadSystem->PauseThread(worker, p_pause);
	}

	m_paused.store(p_pause);
}

bool JobSystem::GetWorkersPaused()
{
	return m_paused.load();
}

size_t JobSystem::GetWorkerNum()
{
	return m_workers.size();
}

void JobSystem::WaitAll()
{
	for (auto& worker : m_workers)
		worker->WaitForIdle();
}

void JobState::Wait()
{
	if (!m_isDone.load())
	{
		std::unique_lock lock{ m_finishedLock };
		m_finishedCondition.wait(lock);
	}
}

bool JobState::IsDone()
{
	return m_isDone.load();
}
