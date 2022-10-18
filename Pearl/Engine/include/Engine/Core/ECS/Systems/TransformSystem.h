#pragma once
#include "Core/ECS/BaseSystem.h"

namespace PrCore::ECS {

	class HierarchyTransform: public BaseSystem {
	public:
		HierarchyTransform() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};
}