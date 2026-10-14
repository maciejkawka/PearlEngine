#pragma once

#include "Engine/Core/ECS/BaseSystem.h"

namespace ChessGame {

	// Prepares the loaded chess scene once on create, register as the first game system
	class CreatorSystem : public PrCore::BaseSystem {
	public:
		CreatorSystem() = default;
		~CreatorSystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override {}

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}
	};
}
