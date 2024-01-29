#pragma once
#include"Core/Utils/Singleton.h"

#include<chrono>

namespace PrCore::Utils {

	class Clock : public Singleton<Clock> {
	public:
		Clock();
		~Clock() = default;

		float GetDeltaTime();

		float GetUnscaledDeltaTime();

		float GetTimeScale();

		float GetRealTime();

		int GetFPS();

		void SetTimeScale(float p_timeScale);

		void SetMaxDeltaTime(float p_maxDeltaTime);

		void SetDefaultDeltaTime(float p_defaultDeltaTime);

		void Tick();

	private:

		std::chrono::time_point <std::chrono::steady_clock> m_beginning;
		std::chrono::time_point <std::chrono::steady_clock> m_frameStart;

		float m_deltaTime;
		float m_realTime;
		float m_timeScale;
		float m_maxDeltaTime;
		float m_defaultDeltaTime;
	};

	class Timer {
	public:
		Timer() = default;
		~Timer() = default;

		void  Start() { m_start = std::chrono::high_resolution_clock::now(); }
		float Stop()
		{
			m_stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::seconds>(m_stop - m_start).count();
		}

	private:
		std::chrono::time_point <std::chrono::high_resolution_clock> m_start;
		std::chrono::time_point <std::chrono::high_resolution_clock> m_stop;
	};

	class HighResolutionTimer {
	public:
		HighResolutionTimer() = default;
		~HighResolutionTimer() = default;

		void  Start() { m_start = std::chrono::high_resolution_clock::now(); }
		float Stop()
		{
			m_stop = std::chrono::high_resolution_clock::now();
			return std::chrono::duration_cast<std::chrono::microseconds>(m_stop - m_start).count() / 1000.0f;
		}

	private:
		std::chrono::time_point <std::chrono::high_resolution_clock> m_start;
		std::chrono::time_point <std::chrono::high_resolution_clock> m_stop;
	};

	class StopeTimer {
	public:
		StopeTimer() = delete;

		explicit StopeTimer(std::function<void(float)> p_callback) :
			m_callback(p_callback)
		{
			m_timer.Start();
		}

		~StopeTimer()
		{
			m_callback(m_timer.Stop());
		}

	private:
		Timer                      m_timer;
		std::function<void(float)> m_callback;
	};

	class HighResolutionStopeTimer {
	public:
		HighResolutionStopeTimer() = delete;

		explicit HighResolutionStopeTimer(std::function<void(float)> p_callback) :
			m_callback(p_callback)
		{
			m_timer.Start();
		}

		~HighResolutionStopeTimer()
		{
			m_callback(m_timer.Stop());
		}

	private:
		HighResolutionTimer        m_timer;
		std::function<void(float)> m_callback;
	};

#define SCOPE_HIGH_TIMER_CALLBACK(functionBody) PrCore::Utils::HighResolutionStopeTimer ScopeTimer([&](float time)  functionBody )
#define SCOPE_TIMER_CALLBACK(functionBody) PrCore::Utils::StopeTimer ScopeTimer([&](float time)  functionBody )

}