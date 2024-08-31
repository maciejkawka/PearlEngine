#include"Core/Common/pearl_pch.h"

#include"Core/Utils/Clock.h"

using namespace PrCore::Utils;
using namespace std::chrono;

Clock::Clock()
{
	m_timeScale = 1.0f;
	m_maxDeltaTime = 2.0f;
	m_defaultDeltaTime = 1.0f / 30.0f;
	m_deltaTime = m_defaultDeltaTime;
	m_realTime = 0.0f;

	m_beginning = steady_clock::now();
	m_frameStart = m_beginning;
}

float Clock::GetDeltaTime() const
{
	return m_deltaTime * m_timeScale;
}

float Clock::GetUnscaledDeltaTime() const
{
	return m_deltaTime;
}

float Clock::GetTimeScale() const
{
	return m_timeScale;
}

float Clock::GetRealTime() const
{
	return m_realTime;
}

int Clock::GetFPS() const
{
	return 1 / m_deltaTime;
}

void Clock::SetTimeScale(float p_timeScale)
{
	m_timeScale = p_timeScale;
}

void Clock::SetMaxDeltaTime(float p_maxDeltaTime)
{
	m_maxDeltaTime = p_maxDeltaTime;
}

void Clock::SetDefaultDeltaTime(float p_defaultDeltaTime)
{
	m_defaultDeltaTime = p_defaultDeltaTime;
}

void Clock::Tick()
{
	auto timeNow = steady_clock::now();

	auto timeSinceBegining = duration<float>(timeNow - m_beginning);
	m_realTime = timeSinceBegining.count();

	m_deltaTime = duration<float>(timeNow - m_frameStart).count();
	if (m_deltaTime > m_maxDeltaTime)
		m_deltaTime = m_defaultDeltaTime;

	m_frameStart = timeNow;
}


