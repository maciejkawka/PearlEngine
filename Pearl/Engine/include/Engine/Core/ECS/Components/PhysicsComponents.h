#pragma once

#include "Core/ECS/BaseComponent.h"

#include "Physics/Actor/IRigidBody.h"

namespace PrCore::ECS {

	class RigidBodyDynamicComponent : public BaseComponent {
	public:
		PrPhysics::IRigidBodyDynamicPtr rigidBody;

		void OnSerialize(Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};

	class RigidBodyStaticComponent : public BaseComponent {
	public:
		PrPhysics::IRigidStaticPtr rigidBody;

		void OnSerialize(Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};
}