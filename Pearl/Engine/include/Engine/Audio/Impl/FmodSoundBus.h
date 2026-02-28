#pragma once

#include "Audio/Core/ISoundBus.h"

namespace FMOD::Studio {
	class Bus;
}

namespace PrAudio
{
	class FmodSoundBus final : public ISoundBus {
	public:
		FmodSoundBus(FMOD::Studio::Bus* p_bus);
		~FmodSoundBus() = default;

		void SetPaused(bool p_paused) override;
		bool GetPaused()override;
		void StopAllEvents(StopMode p_stopMode) override;

		void SetMute(bool p_mute) override;
		bool GetMute() override;

		float GetVolume() override;
		void  SetVolume(float p_volume) override;

		const std::string& GetPath() override;

	private:
		FMOD::Studio::Bus* m_bus;
		std::string        m_path;
	};
}