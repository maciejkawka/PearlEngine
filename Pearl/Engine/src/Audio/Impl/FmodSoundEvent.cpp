#include "Core/Common/pearl_pch.h"

#include "Audio/Impl/FmodSoundEvent.h"
#include "Audio/Impl/FmodAudioSystem.h"
#include "Audio/Impl/ConvertUtils.h"

#include "fmod/fmod_studio.hpp"

namespace PrAudio {

	FmodSoundEvent::FmodSoundEvent(FMOD::Studio::EventDescription* p_description, bool p_preload) :
		m_eventDescription(p_description),
		m_eventInstance(nullptr)
	{
		PR_ASSERT(m_eventDescription, "Event description is nullptr");

		if (p_preload)
			m_eventDescription->loadSampleData();

		m_eventDescription->isStream(&m_isStream);
		m_eventDescription->is3D(&m_is3d);
		m_eventDescription->isSnapshot(&m_isSnapShot);
		m_eventDescription->isDopplerEnabled(&m_isDopplerEnabled);
		m_eventDescription->getSoundSize(&m_size);
		m_eventDescription->getMinMaxDistance(&m_minMax.first, &m_minMax.second);
	}

	FmodSoundEvent::~FmodSoundEvent()
	{
		m_eventDescription->unloadSampleData();

		if (m_eventInstance)
			m_eventInstance->release();
	}

	void FmodSoundEvent::PlayOneShot()
	{
		FMOD::Studio::EventInstance* oneShotInstance = nullptr;
		m_eventDescription->createInstance(&oneShotInstance);

		if (oneShotInstance)
		{
			auto fmodAtributtes = ToFmod(m_3dAttributes);
			oneShotInstance->set3DAttributes(&fmodAtributtes);

			for (auto& [name, value] : m_parametersMap)
			{
				oneShotInstance->setParameterByName(name.c_str(), value);
			}

			oneShotInstance->set3DAttributes(&fmodAtributtes);
			oneShotInstance->start();
			oneShotInstance->release();
		}
	}

	void FmodSoundEvent::Play()
	{
		if (!m_eventInstance)
		{
			Preload();
		}

		if (m_eventInstance)
		{
			auto attributes = ToFmod(m_3dAttributes);
			m_eventInstance->set3DAttributes(&attributes);
			for (auto& [name, value] : m_parametersMap)
			{
				m_eventInstance->setParameterByName(name.c_str(), value);
			}

			m_eventInstance->start();
		}
	}

	void FmodSoundEvent::Stop(StopMode p_stopMode)
	{
		if (m_eventInstance)
		{
			m_eventInstance->stop(ToFmod(p_stopMode));
		}
	}

	void FmodSoundEvent::Preload()
	{
		if (!m_eventInstance)
		{
			m_eventDescription->createInstance(&m_eventInstance);
		}
	}

	void FmodSoundEvent::SetPaused(bool p_paused)
	{
		if (m_eventInstance)
		{
			m_eventInstance->setPaused(p_paused);
		}
	}

	bool FmodSoundEvent::GetPaused()
	{
		if (m_eventInstance)
		{
			bool paused = false;
			m_eventInstance->getPaused(&paused);
			return paused;
		}

		return true;
	}

	void FmodSoundEvent::SetPitch(float p_pitch)
	{
		if (m_eventInstance)
		{
			m_eventInstance->setPitch(p_pitch);
		}

		m_pitch = p_pitch;
	}

	float FmodSoundEvent::GetPitch()
	{
		return m_pitch;
	}

	float FmodSoundEvent::GetVolume()
	{
		return m_volume;
	}

	void FmodSoundEvent::SetVolume(float p_volume)
	{
		if (m_eventInstance)
		{
			m_eventInstance->setVolume(p_volume);
		}

		m_volume = p_volume;
	}

	float FmodSoundEvent::GetParameter(std::string_view p_name)
	{
		auto it = m_parametersMap.find(std::string{ p_name });

		if (it == m_parametersMap.end())
		{
			PRLOG_ERROR("Cannot find EventInstance parameter {}", p_name);
			return 0.0f;
		}

		return it->second;
	}

	void FmodSoundEvent::SetParameter(std::string_view p_name, float p_val)
	{
		std::string strName{ p_name };
		if (m_eventInstance)
		{
			m_eventInstance->setParameterByName(strName.c_str(), p_val);
		}

		m_parametersMap[strName] = p_val;
	}

	const Attributes3D& FmodSoundEvent::Get3DAttributes()
	{
		return m_3dAttributes;
	}

	void FmodSoundEvent::Set3DAttributes(Attributes3D& p_3DAttributes)
	{
		if (m_eventInstance)
		{
			auto attributes = ToFmod(p_3DAttributes);
			m_eventInstance->set3DAttributes(&attributes);
		}
		m_3dAttributes = p_3DAttributes;
	}

	void FmodSoundEvent::SetReverbLevel(int p_index, float p_level)
	{
		PR_ASSERT(p_index < 4, "Rever index must be 0-4");
		if (m_eventInstance)
		{
			m_eventInstance->setReverbLevel(p_index, p_level);
		}
		m_reverb[p_index] = p_level;
	}

	float FmodSoundEvent::GetReverbLevel(int p_index)
	{
		PR_ASSERT(p_index < 4, "Rever index must be 0-4");
		return m_reverb[p_index];
	}

	bool FmodSoundEvent::IsStream()
	{
		return m_isStream;
	}

	bool FmodSoundEvent::Is3D()
	{
		return m_is3d;
	}

	bool FmodSoundEvent::IsSnapShot()
	{
		return m_isSnapShot;
	}

	bool FmodSoundEvent::IsDopplerEnabled()
	{
		return m_isDopplerEnabled;
	}

	void FmodSoundEvent::GetMaxMinDistance(float& p_min, float& p_max)
	{
		p_min = m_minMax.first;
		p_max = m_minMax.second;
	}

	float FmodSoundEvent::GetSize()
	{
		return m_size;
	}
}