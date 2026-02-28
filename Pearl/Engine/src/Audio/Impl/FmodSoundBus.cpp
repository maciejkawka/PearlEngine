#include "Core/Common/pearl_pch.h"

#include "Audio/Core/IAudioSystem.h"
#include "Audio/Impl/FmodSoundBus.h"
#include "Audio/Impl/ConvertUtils.h"

#include "fmod/fmod_studio.hpp"

using namespace PrAudio;

FmodSoundBus::FmodSoundBus(FMOD::Studio::Bus* p_bus) :
	m_bus(p_bus)
{
	m_path.resize(64);
	int shrinkSize;
	m_bus->getPath(m_path.data(), 64, &shrinkSize);
	m_path.resize(shrinkSize);
}

void FmodSoundBus::SetPaused(bool p_paused)
{
	m_bus->setPaused(p_paused);
}

bool FmodSoundBus::GetPaused()
{
	bool paused;
	m_bus->getPaused(&paused);

	return paused;
}
void FmodSoundBus::StopAllEvents(StopMode p_stopMode)
{
	m_bus->stopAllEvents(ToFmod(p_stopMode));
}

void FmodSoundBus::SetMute(bool p_mute)
{
	m_bus->setMute(p_mute);
}

bool FmodSoundBus::GetMute()
{
	bool mute;
	m_bus->getMute(&mute);
	return mute;
}

float FmodSoundBus::GetVolume()
{
	float volume;
	m_bus->getVolume(&volume);
	return volume;
}

void FmodSoundBus::SetVolume(float p_volume)
{
	m_bus->setVolume(p_volume);
}

const std::string& FmodSoundBus::GetPath()
{
	return m_path;
}