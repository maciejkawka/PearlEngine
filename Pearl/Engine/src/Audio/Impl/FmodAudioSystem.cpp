#include "Core/Common/pearl_pch.h"

#include "Core/File/FileSystem.h"

#include "Audio/Impl/FmodAudioSystem.h"
#include "Audio/Impl/FmodSoundEvent.h"
#include "Audio/Impl/FmodSoundBus.h"
#include "Audio/Impl/ConvertUtils.h"

#include "fmod/fmod_studio.hpp"

namespace PrAudio {

	FmodAudioSystem::FmodAudioSystem()
	{
		FMOD_RESULT result;
		result = FMOD::Studio::System::create(&m_studioSystem);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		result = m_studioSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		result = m_studioSystem->getCoreSystem(&m_coreSystem);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		///////////////////////////////////////
		// Test
		auto assetPath = PrSystems::Get<PrCore::FileSystem>()->GetGameAssetsPath();
		auto makePath = PrCore::PathUtils::MakePath(assetPath, "Master Bank.bank");
		auto makePathString = PrCore::PathUtils::MakePath(assetPath, "Master Bank.strings.bank");

		FMOD::Studio::Bank* masterBank = NULL;
		m_studioSystem->loadBankFile(makePath.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &masterBank);

		FMOD::Studio::Bank* stringBank = NULL;
		m_studioSystem->loadBankFile(makePathString.c_str(), FMOD_STUDIO_LOAD_BANK_NORMAL, &stringBank);

		//FMOD::Studio::EventDescription* eventDescription = NULL;
		//m_studioSystem->getEvent("event:/Player/Song", &eventDescription);

		//FMOD::Studio::EventInstance* eventInstance = NULL;
		//eventDescription->createInstance(&eventInstance);

		//eventInstance->setVolume(0.1f);
		//eventInstance->start();
		///////////////////////////////////////
	}

	FmodAudioSystem::~FmodAudioSystem()
	{
		m_studioSystem->release();
		m_studioSystem = nullptr;
		m_coreSystem = nullptr;
	}

	void FmodAudioSystem::Update()
	{
		m_studioSystem->update();
	}

	DriverInfo FmodAudioSystem::GetDriver(int p_index)
	{
		DriverInfo info;
		info.id = p_index;
		info.name.resize(64);

		FMOD_GUID guid;
		FMOD_SPEAKERMODE speakerMode;
		m_coreSystem->getDriverInfo(p_index, info.name.data(), info.name.length(), &guid, &info.systemRate, &speakerMode, &info.speakerModeChannels);
		info.speakerMode = FromFmod(speakerMode);

		return info;
	}

	DriverInfo FmodAudioSystem::GetCurrentDriver()
	{
		int driverId = -1;
		if (m_coreSystem->getDriver(&driverId) != FMOD_OK)
		{
			PR_ASSERT(false, "Cannot get driver!");
			return DriverInfo{};
		}

		return GetDriver(driverId);
	}

	void FmodAudioSystem::GetAllDrivers(std::vector<DriverInfo>& p_drivers)
	{
		PR_ASSERT(p_drivers.empty(), "Driver list is not empty!");

		int driversNum;
		m_coreSystem->getNumDrivers(&driversNum);
		for (int i = 0; i < driversNum; i++)
		{
			p_drivers.push_back(GetDriver(i));
		}
	}

	void FmodAudioSystem::SetDriver(int p_index)
	{
		int driversNum;
		m_coreSystem->getNumDrivers(&driversNum);
		if (p_index < driversNum)
		{
			m_coreSystem->setDriver(p_index);
		}
		else
		{
			PR_ASSERT("Cannot change the audio driver!");
		}
	}

	void FmodAudioSystem::SetListenerAttributes(Attributes3D& p_listenerAttributes)
	{
		auto fmodAttributes = ToFmod(p_listenerAttributes);
		m_studioSystem->setListenerAttributes(0, &fmodAttributes);
	}

	float FmodAudioSystem::GetGlobalParameter(const std::string& p_name)
	{
		float value = 0;
		m_studioSystem->getParameterByName(p_name.c_str(), &value);

		return value;
	}

	void FmodAudioSystem::SetGlobalParameter(const std::string& p_name, float p_value)
	{
		m_studioSystem->setParameterByName(p_name.c_str(), p_value);
	}

	ISoundEventPtr FmodAudioSystem::CreateSoundEvent(std::string_view p_path, bool p_preload)
	{
		FMOD::Studio::EventDescription* eventDescription;
		m_studioSystem->getEvent(p_path.data(), &eventDescription);

		if (!eventDescription)
			return nullptr;

		auto soundEvent = std::make_shared<FmodSoundEvent>(eventDescription, p_preload);
		return soundEvent;
	}

	ISoundBusPtr FmodAudioSystem::GetSoundBus(std::string_view p_path)
	{
		FMOD::Studio::Bus* soundBus;
		m_studioSystem->getBus(p_path.data(), &soundBus);

		if (!soundBus)
			return nullptr;

		auto soundBusPtr = std::make_shared<FmodSoundBus>(soundBus);
		return soundBusPtr;
	}
}