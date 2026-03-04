#pragma once

#include "Core/ECS/BaseComponent.h"
#include "Audio/Core/ISoundEvent.h"

namespace PrCore {

	class AudioSourceComponent : public BaseComponent {
	public:
		PrAudio::ISoundEventPtr audioEvent;

		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
		}
	};

	class AudioListenerComponent : public BaseComponent {
	public:
		void OnSerialize(Utils::JSON::json& p_serialized) override
		{
		}

		void OnDeserialize(const Utils::JSON::json& p_deserialized) override
		{
		}
	};
}