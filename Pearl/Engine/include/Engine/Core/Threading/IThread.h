#pragma once

namespace PrCore::Threading {

	class ThreadSystem;

	class IThread {
	public:
		virtual int ThreadLoop() = 0;

	protected:
		// Use these functions in the thread loop to add pause and terminate
		virtual bool ShouldPause() = 0;
		virtual bool ShouldTerminate() = 0;

		// Exposed pause and terminate functionality
		// This is only exposed to the ThreadSystem
		virtual void SetPaused(bool isPaused) = 0;
		virtual void Terminate() = 0;

		virtual bool IsPaused() = 0;
		virtual bool IsTerminated() = 0;

		friend class ThreadSystem;
	};
	using IThreadPtr = std::shared_ptr<IThread>;
}