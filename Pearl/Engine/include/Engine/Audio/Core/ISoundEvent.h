#pragma once

#include "Types.h"

namespace PrAudio {

	class ISoundEvent {
	public:
		virtual ~ISoundEvent() = default;

		virtual void PlayOneShot() = 0;
		virtual void Play() = 0;
		virtual void Stop(StopMode p_stopMode) = 0;

		virtual void Preload() = 0;

		virtual void SetPaused(bool p_paused) = 0;
		virtual bool GetPaused() = 0;

		virtual void  SetPitch(float p_pitch) = 0;
		virtual float GetPitch() = 0;

		virtual float GetVolume() = 0;
		virtual void  SetVolume(float p_volume) = 0;

		virtual float GetParameter(std::string_view p_name) = 0;
		virtual void  SetParameter(std::string_view p_name, float p_val) = 0;

		virtual const Attributes3D& Get3DAttributes() = 0;
		virtual void                Set3DAttributes(Attributes3D& p_3DAttributes) = 0;

		virtual void  SetReverbLevel(int p_index, float p_level) = 0;
		virtual float GetReverbLevel(int p_index) = 0;

		virtual bool  IsPlaying() = 0;

		virtual bool  IsStream() = 0;
		virtual bool  Is3D() = 0;
		virtual bool  IsSnapShot() = 0;
		virtual bool  IsDopplerEnabled() = 0;
		virtual void  GetMaxMinDistance(float& p_min, float& p_max) = 0;
		virtual float GetSize() = 0;
	};
	using ISoundEventPtr = std::shared_ptr<ISoundEvent>;
}