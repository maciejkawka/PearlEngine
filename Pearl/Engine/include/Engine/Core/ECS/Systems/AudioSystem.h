#pragma once

#include "Core/ECS/BaseSystem.h"
#include "Core/ECS/Components.h"

namespace PrCore {

	class AudioSystem : public BaseSystem {
	public:
		AudioSystem() = default;
		~AudioSystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnListenerEnabled(PrCore::EventPtr p_event);

		virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}

	private:
		TransformComponent*        m_activeListenerTransform;
		RigidBodyDynamicComponent* m_activeListenerRigibBody;
	};
}