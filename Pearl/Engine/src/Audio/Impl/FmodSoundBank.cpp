#include "Core/Common/pearl_pch.h"

#include "Audio/Impl/FmodSoundBank.h"

#include "fmod/fmod_studio.hpp"

using namespace PrAudio;

FmodSoundBank::FmodSoundBank(FMOD::Studio::Bank* p_bank) :
	m_bank(p_bank)
{
	PR_ASSERT(p_bank, "Bank is nullptr");

	m_path.resize(64);
	int shrinkSize;
	p_bank->getPath(m_path.data(), 64, &shrinkSize);
	m_path.resize(shrinkSize);

	// Get Events
	int eventCount;
	m_bank->getEventCount(&eventCount);
	if (eventCount > 0)
	{
		std::vector<FMOD::Studio::EventDescription*> events;
		events.resize(eventCount);
		int actualSize = 0;
		m_bank->getEventList(events.data(), eventCount, &actualSize);
		PR_ASSERT(actualSize == eventCount);

		std::string name;
		name.resize(64);
		for (auto soundEvent : events)
		{
			int stringSize = 0;
			soundEvent->getPath(name.data(), 64, &stringSize);
			m_events.emplace_back(name.c_str(), stringSize);
		}
	}

	// Get Busses
	int busCount;
	m_bank->getBusCount(&busCount);
	if (busCount > 0)
	{
		std::vector<FMOD::Studio::Bus*> buses;
		buses.resize(busCount);
		int actualSize = 0;
		m_bank->getBusList(buses.data(), busCount, &actualSize);
		PR_ASSERT(actualSize == busCount);

		std::string name;
		name.resize(64);
		for (auto soundBus : buses)
		{
			int stringSize = 0;
			soundBus->getPath(name.data(), 64, &stringSize);
			m_events.emplace_back(name.c_str(), stringSize);
		}
	}
}

std::string_view FmodSoundBank::GetPath()
{
	return m_path;
}

void FmodSoundBank::PreloadAll()
{
	m_bank->loadSampleData();
}

void FmodSoundBank::UnloadAll()
{
	m_bank->unload();
}

int FmodSoundBank::GetEventCount()
{
	int eventCount = 0;
	m_bank->getEventCount(&eventCount);
	return eventCount;
}

int FmodSoundBank::GetBusCount()
{
	int busCount = 0;
	m_bank->getBusCount(&busCount);
	return busCount;
}

const std::vector<std::string>& FmodSoundBank::GetEventList() const
{
	return m_events;
}

const std::vector<std::string>& FmodSoundBank::GetBusList() const
{
	return m_buses;
}

size_t FmodSoundBank::GetByteSize() const
{
	return 0;
}