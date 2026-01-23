#pragma once

#include "Core/ECS/BaseSystem.h"
#include "Physics/Core/PhysicsSystem.h"

namespace PrCore::ECS {

	class PhysicsUpdateSystem : public BaseSystem {
	public:
		PhysicsUpdateSystem();
		~PhysicsUpdateSystem() override = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}

		void OnComponentDynamicCreated(PrCore::EventPtr p_eventType);
		void OnComponentDynamicRemoved(PrCore::EventPtr p_eventType);
		
		void OnComponentStaticCreated(PrCore::EventPtr p_eventType);
		void OnComponentStaticRemoved(PrCore::EventPtr p_eventType);

	private:
		PrPhysics::PhysicsSystem*         m_physics;

		std::vector<PrPhysics::IActorPtr> m_createdActors;
		std::vector<PrPhysics::IActorPtr> m_removedActors;
	};
}