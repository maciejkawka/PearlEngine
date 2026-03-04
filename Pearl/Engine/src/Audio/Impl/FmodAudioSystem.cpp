#include "Core/Common/pearl_pch.h"

#include "Core/File/FileSystem.h"

#include "Audio/Impl/FmodAudioSystem.h"
#include "Audio/Impl/FmodSoundEvent.h"
#include "Audio/Impl/FmodSoundBus.h"
#include "Audio/Impl/FmodSoundBank.h"
#include "Audio/Impl/ConvertUtils.h"

#include "fmod/fmod_studio.hpp"
#include "fmod/fmod.h"

#include "Core/File/FileSystem.h"
namespace PrAudio {

	//////////////////////
	// File IO Callbacks
	FMOD_RESULT F_CALL FmodOpenCallback(
		const char* name,
		unsigned int* filesize,
		void** handle,
		void* userdata
	)
	{
		auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
		PrCore::FileHandle file = pFileSystem->FileOpen(name);
		if (!file)
			return FMOD_ERR_FILE_NOTFOUND;

		*filesize = pFileSystem->FileSize(file);
		*handle = file;
		return FMOD_OK;
	}

	FMOD_RESULT F_CALL FmodCloseCallback(void* handle, void* userdata)
	{
		auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
		pFileSystem->FileClose(handle);
		return FMOD_OK;
	}

	FMOD_RESULT F_CALL FmodReadCallback(
		void* handle,
		void* buffer,
		unsigned int sizebytes,
		unsigned int* bytesread,
		void* userdata
	)
	{
		auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
		*bytesread = pFileSystem->FileRead(handle, buffer, sizebytes);

		if (*bytesread < sizebytes)
		{
			if (pFileSystem->FileEOF(handle))
			{
				return FMOD_ERR_FILE_EOF;
			}
		}

		return FMOD_OK;
	}

	FMOD_RESULT F_CALL FmodSeekCallback(void* handle, unsigned int pos, void* userdata)
	{
		PrSystems::Get<PrCore::FileSystem>()->FileSeek(handle, pos);
		return FMOD_OK;
	}
	//////////////////////

	FmodAudioSystem::FmodAudioSystem()
	{
		FMOD_RESULT result;
		result = FMOD::Studio::System::create(&m_studioSystem);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		result = m_studioSystem->initialize(512, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, 0);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		result = m_studioSystem->getCoreSystem(&m_coreSystem);
		if (result != FMOD_OK)
		{
			PRLOG_ERROR("FMOD error");
		}

		// Set File IO Callbacks
		m_coreSystem->setFileSystem(
			FmodOpenCallback,
			FmodCloseCallback,
			FmodReadCallback,
			FmodSeekCallback,
			nullptr,
			nullptr,
			4096
		);
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
		// Inverse forward vector, FMOD forward is +Z whereas Pearl Engine is -Z
		p_listenerAttributes.forwardVec = -p_listenerAttributes.forwardVec;

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

	ISoundBankPtr FmodAudioSystem::LoadSoundBank(std::string_view p_path)
	{
		FMOD::Studio::Bank* bank;
		m_studioSystem->loadBankFile(p_path.data(), FMOD_STUDIO_LOAD_BANK_NORMAL, &bank);

		if (!bank)
			return nullptr;

		auto soundBankPtr = std::make_shared<FmodSoundBank>(bank);
		return soundBankPtr;
	}
}