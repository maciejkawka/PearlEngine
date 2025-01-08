#pragma once

#include "IThread.h"
#include "Core/Utils/Singleton.h"

#include <shared_mutex>
#include <string>

namespace PrCore::Threading {

	// Basic thread info stored in the ThreadSystem
	struct ThreadInfo
	{
		std::string name = "";
		uint64_t    affinityMask = 0;
		uint64_t    stackSize = 0;
		uint64_t    threadId = 0;
	};

	// Use to create the thread
	struct ThreadConfig
	{
		std::string name = "PearlThread";
		uint64_t    affinityMask = 0xFFFFFFFF;
		uint64_t    stackSize = 0;
		bool        startPaused = false;
	};

	class ThreadSystem : public Utils::Singleton<ThreadSystem> {
	public:
		~ThreadSystem();

		bool SpawnThread(IThreadPtr p_thread, const ThreadConfig& p_desc);
		void JoinThread(IThreadPtr p_thread);

		void PauseThread(IThreadPtr p_thread, bool p_pause);
		void TerminateThread(IThreadPtr p_thread);

		bool IsThreadPaused(IThreadPtr p_thread);
		bool IsThreadTerminated(IThreadPtr p_thread);

		size_t     GetThreadNum();
		ThreadInfo GetThreadInfo(IThreadPtr p_thread);

	private:
		struct ThreadDesc
		{
			IThreadPtr  threadPtr = nullptr;
			std::string name = "PearlThread";
			uint64_t    affinityMask = 0xFFFFFFFF;
			uint64_t    stackSize = 0;

			void*       nativeHandle = 0;
			uint64_t    threadId = 0;
		};

		struct ThreadDescComparator
		{
			using is_transparent = void;

			bool operator()(const ThreadDesc& lhs, const ThreadDesc& rhs) const
			{
				return lhs.threadPtr < rhs.threadPtr;
			}

			bool operator()(const ThreadDesc& lhs, const IThreadPtr& rhs) const
			{
				return lhs.threadPtr.get() < rhs.get();
			}

			bool operator()(const IThreadPtr& lhs, const ThreadDesc& rhs) const
			{
				return lhs.get() < rhs.threadPtr.get();
			}
		};

		void* GetThreadHandle(IThreadPtr p_thread);

		std::set<ThreadDesc, ThreadDescComparator>  m_activeThreads;
		std::shared_mutex                           m_activeThreadLock;

		friend DWORD WINAPI ThreadEntry(LPVOID p_thread);
	};
}
