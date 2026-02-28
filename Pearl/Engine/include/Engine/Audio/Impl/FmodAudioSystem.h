#pragma once

#include "Audio/Core/IAudioSystem.h"

#include "Core/Events/Event.h"

namespace FMOD
{
	class System;

	namespace Studio
	{
		class System;
	}
}

namespace PrAudio {

	class FmodAudioSystem final : public IAudioSystem {
	public:
		FmodAudioSystem();
		~FmodAudioSystem() override;

		void Update() override;

		DriverInfo      GetDriver(int p_index) override;
		DriverInfo      GetCurrentDriver() override;
		void            GetAllDrivers(std::vector<DriverInfo>& p_drivers) override;
		void            SetDriver(int p_index) override;

		void            SetListenerAttributes(Attributes3D& p_listenerAttributes) override;

		float GetGlobalParameter(const std::string& p_name) override;
		void  SetGlobalParameter(const std::string& p_name, float p_value) override;

		ISoundEventPtr CreateSoundEvent(std::string_view p_path, bool p_preload = false) override;
		ISoundBusPtr GetSoundBus(std::string_view p_path) override;

	private:
		FMOD::Studio::System* m_studioSystem;
		FMOD::System* m_coreSystem;
	};
}