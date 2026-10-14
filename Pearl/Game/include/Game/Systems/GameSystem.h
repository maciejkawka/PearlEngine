#pragma once

#include "Engine/Core/ECS/BaseSystem.h"

namespace ChessGame {

	class BoardCoordinator;

	// Owns the BoardCoordinator lifetime start, handles game input and synchronizes the board
	// Register as the last game system so moves made in the frame are synchronized in the same frame
	class GameSystem : public PrCore::BaseSystem {
	public:
		GameSystem() = default;
		~GameSystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		BoardCoordinator* m_coordinator = nullptr;
	};
}
