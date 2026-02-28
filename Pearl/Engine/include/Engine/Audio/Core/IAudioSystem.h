#pragma once

#include "Types.h"

#include "Core/Utils/ISystem.h"

namespace PrAudio {

	class ISoundEvent;
	using ISoundEventPtr = std::shared_ptr<ISoundEvent>;

	class ISoundBus;
	using ISoundBusPtr = std::shared_ptr<ISoundBus>;

	class ISoundBank;
	using ISoundBankPtr = std::shared_ptr<ISoundBank>;

	class IAudioSystem : public PrCore::Utils::ISystem {
	public:
		virtual ~IAudioSystem() = default;

		virtual void            Update() = 0;

		virtual DriverInfo      GetDriver(int p_index) = 0;
		virtual DriverInfo      GetCurrentDriver() = 0;
		virtual void            GetAllDrivers(std::vector<DriverInfo>& p_drivers) = 0;
		virtual void            SetDriver(int p_index) = 0;

		virtual void            SetListenerAttributes(Attributes3D& p_listenerAttributes) = 0;

		virtual float GetGlobalParameter(const std::string& p_name) = 0;
		virtual void  SetGlobalParameter(const std::string& p_name, float p_value) = 0;

		virtual ISoundEventPtr CreateSoundEvent(std::string_view p_path, bool p_preload = false) = 0;
		virtual ISoundBusPtr   GetSoundBus(std::string_view p_path) = 0;
		virtual ISoundBankPtr  LoadSoundBank(std::string_view p_path) = 0;
		
		// Implement later, probably never :D
		//virtual ISoundEvent  CreatesoundClip(std::string_view p_path, bool p_preload = false) = 0; In future
	};
}