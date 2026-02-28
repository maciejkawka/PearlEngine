#pragma once

#include "Types.h"

namespace PrAudio {

	class ISoundBus {
	public:
		virtual ~ISoundBus() = default;

		virtual void SetPaused(bool p_paused) = 0;
		virtual bool GetPaused() = 0;
		virtual void StopAllEvents(StopMode p_stopMode) = 0;

		virtual void SetMute(bool p_mute) = 0;
		virtual bool GetMute() = 0;

		virtual float GetVolume() = 0;
		virtual void  SetVolume(float p_volume) = 0;

		virtual const std::string& GetPath() = 0;
	};
	using ISoundBusPtr = std::shared_ptr<ISoundBus>;
}