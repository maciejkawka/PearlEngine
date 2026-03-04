#pragma once

#include "Audio/Core/ISoundEvent.h"

#include <utility>

namespace FMOD::Studio {
	class EventDescription;
	class EventInstance;
}

namespace PrAudio {

	class FmodSoundEvent final : public ISoundEvent {
	public:
		FmodSoundEvent(FMOD::Studio::EventDescription* p_description, bool p_preload = false);
		~FmodSoundEvent() override;

		void PlayOneShot() override;
		void Play() override;
		void Stop(StopMode p_stopMode) override;

		void Preload() override;

		void SetPaused(bool p_paused) override;
		bool GetPaused() override;

		void  SetPitch(float p_pitch) override;
		float GetPitch() override;

		float GetVolume() override;
		void  SetVolume(float p_volume) override;

		float GetParameter(std::string_view p_name) override;
		void  SetParameter(std::string_view p_name, float p_val) override;

		const Attributes3D& Get3DAttributes() override;
		void                Set3DAttributes(Attributes3D& p_3DAttributes) override;

		void  SetReverbLevel(int p_index, float p_level) override;
		float GetReverbLevel(int p_index) override;

		bool  IsPlaying() override;

		bool  IsStream() override;
		bool  Is3D() override;
		bool  IsSnapShot() override;
		bool  IsDopplerEnabled() override;
		void  GetMaxMinDistance(float& p_min, float& p_max) override;
		float GetSize() override;

	private:
		using ParametersMap = std::unordered_map<std::string, float>;
		using ReverbVec = float[4];
		using MinMax = std::pair<float,float>;

		bool                  m_isStream;
		bool                  m_is3d;
		bool                  m_isSnapShot;
		bool                  m_isDopplerEnabled;
		float                 m_size;
		MinMax                m_minMax;

		float                 m_pitch;
		float                 m_volume;
		ReverbVec             m_reverb;
		ParametersMap         m_parametersMap;
		Attributes3D          m_3dAttributes;

		FMOD::Studio::EventDescription* m_eventDescription = nullptr;
		FMOD::Studio::EventInstance*    m_eventInstance = nullptr;
	};
}