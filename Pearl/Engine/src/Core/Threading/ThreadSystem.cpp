#include "Core/Common/pearl_pch.h"

#include "Core/Threading/ThreadSystem.h"

#include <windows.h>

namespace PrCore::Threading {
	DWORD WINAPI ThreadEntry(LPVOID p_thread)
	{
		auto threadPtr = *static_cast<IThreadPtr*>(p_thread);
		int exitCode = threadPtr->ThreadLoop();

		// Remove thread from the registry when terminated
		{
			auto threadSystem = ThreadSystem::GetInstancePtr();
			std::lock_guard lock{ threadSystem->m_activeThreadLock };
			auto it = threadSystem->m_activeThreads.find(threadPtr);
			if (it == threadSystem->m_activeThreads.end())
			{
				PRLOG_WARN("Does not exist or already terminated");
			}
			else
			{
				PRLOG_INFO("Terminated thread id: {} name: {} with affinityMask {:b}", it->threadId, it->name, it->affinityMask);
				threadSystem->m_activeThreads.erase(it);
			}
		}

		delete static_cast<IThreadPtr*>(p_thread);

		return exitCode;
	}
}

using namespace PrCore::Threading;

ThreadSystem::~ThreadSystem()
{
	// Terminate all threads
	std::lock_guard lock{ m_activeThreadLock };
	for (auto& threadDesc : m_activeThreads)
	{
		threadDesc.threadPtr->Terminate();
		::WaitForSingleObject(threadDesc.nativeHandle, INFINITE);
		::CloseHandle(threadDesc.nativeHandle);

		PRLOG_INFO("Terminating thread id: {} name: {}", threadDesc.threadId, threadDesc.name);
	}

	m_activeThreads.clear();
}

bool ThreadSystem::SpawnThread(IThreadPtr p_thread, const ThreadConfig& p_config)
{
	PR_ASSERT(p_thread, "Thread is null");

	// Check if thread already spawned
	{
		std::shared_lock lock{ m_activeThreadLock };
		if (m_activeThreads.find(p_thread) != m_activeThreads.end())
		{
			PR_ASSERT(false, "Thread already spawned");
			return false;
		}
	}

	// Set paused before spawning the thread if should start paused
	p_thread->SetPaused(p_config.startPaused);

	// Dynamically allocate the shared_ptr to avoid loosing the stack address, the thread is responsible for deleting the memory
	std::shared_ptr<IThread>* heapSharedPtr = new std::shared_ptr<IThread>(p_thread);
	
	DWORD threadId = 0;
	HANDLE threadHandle = ::CreateThread(NULL,
		p_config.stackSize * 1024,
		PrCore::Threading::ThreadEntry,
		heapSharedPtr,
		0,
		&threadId
	);

	if (threadHandle == NULL)
	{
		PR_ASSERT(threadHandle, "Failed to spawn the thread");
		return false;
	}

	::SetThreadAffinityMask(threadHandle, p_config.affinityMask);
	::SetThreadPriority(threadHandle, THREAD_PRIORITY_NORMAL);

	std::wstring stemp = std::wstring(p_config.name.begin(), p_config.name.end());
	LPCWSTR wstr = stemp.c_str();
	::SetThreadDescription(threadHandle, wstr);

	ThreadDesc threadDesc;
	threadDesc.threadPtr = p_thread;
	threadDesc.name = p_config.name;
	threadDesc.affinityMask = p_config.affinityMask;
	threadDesc.stackSize = p_config.stackSize;
	threadDesc.nativeHandle = threadHandle;
	threadDesc.threadId = threadId;

	PRLOG_INFO("Created thread id: {} name: {} with mask 0x{:b}", threadDesc.threadId, threadDesc.name, threadDesc.affinityMask);
	
	// Insert thread into the register
	{
		std::lock_guard lock{ m_activeThreadLock };
		m_activeThreads.insert(std::move(threadDesc));
	}

	return true;
}

void ThreadSystem::JoinThread(IThreadPtr p_thread)
{
	HANDLE threadHandle = GetThreadHandle(p_thread);
	
	if(threadHandle)
		::WaitForSingleObject(threadHandle, INFINITE);
}

void* ThreadSystem::GetThreadHandle(IThreadPtr p_thread)
{
	std::shared_lock lock{ m_activeThreadLock };

	auto it = m_activeThreads.find(p_thread);
	if (it == m_activeThreads.end())
	{
		return nullptr;
	}
	else
	{
		return it->nativeHandle;
	}
}

void ThreadSystem::PauseThread(IThreadPtr p_thread, bool p_pause)
{
	PR_ASSERT(p_thread, "Thread is null");
	p_thread->SetPaused(p_pause);
}

void ThreadSystem::TerminateThread(IThreadPtr p_thread)
{
	PR_ASSERT(p_thread, "Thread is null");
	p_thread->Terminate();
}

bool ThreadSystem::IsThreadPaused(IThreadPtr p_thread)
{
	PR_ASSERT(p_thread, "Thread is null");
	return p_thread->IsPaused();
}

bool ThreadSystem::IsThreadTerminated(IThreadPtr p_thread)
{
	PR_ASSERT(p_thread, "Thread is null");
	return p_thread->IsTerminated();
}

ThreadInfo ThreadSystem::GetThreadInfo(IThreadPtr p_thread)
{
	PR_ASSERT(p_thread, "Thread is null");
	std::shared_lock lock{ m_activeThreadLock };

	const auto it = m_activeThreads.find(p_thread);
	if (it == m_activeThreads.end())
	{
		PR_ASSERT(false, "Thread does not exust or was terminated");
		return ThreadInfo{};
	}
	else
	{
		ThreadInfo threadInfo;
		threadInfo.name = it->name;
		threadInfo.affinityMask = it->affinityMask;
		threadInfo.stackSize = it->stackSize;
		threadInfo.threadId = it->threadId;
		return threadInfo;
	}
}

size_t ThreadSystem::GetThreadNum()
{
	std::shared_lock lock{ m_activeThreadLock };
	return m_activeThreads.size();
}