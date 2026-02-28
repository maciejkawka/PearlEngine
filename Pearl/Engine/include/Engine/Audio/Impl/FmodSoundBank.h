#pragma once

#include "Audio/Resources/ISoundBank.h"

namespace FMOD::Studio {
	class Bank;
}

namespace PrAudio
{
	class FmodSoundBank final : public ISoundBank {
	public:
		FmodSoundBank(FMOD::Studio::Bank* p_bank);
		~FmodSoundBank() override = default;

		std::string_view GetPath() override;

		void PreloadAll() override;
		void UnloadAll() override;

		int GetEventCount() override;
		int GetBusCount() override;

		const std::vector<std::string>& GetEventList() const override;
		const std::vector<std::string>& GetBusList() const override;

		size_t GetByteSize() const override;

	private:
		FMOD::Studio::Bank* m_bank;
		std::string         m_path;

		std::vector<std::string> m_events;
		std::vector<std::string> m_buses;
	};
}