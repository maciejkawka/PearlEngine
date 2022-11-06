#pragma once
#include<chrono>

namespace PrCore::Utils {

	class Clock {
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


}