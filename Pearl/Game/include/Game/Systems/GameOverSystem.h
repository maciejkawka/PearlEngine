#pragma once

#include "ChessEngine/ChessSystem.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"

namespace ChessGame {

	// Game over feedback on the board, the checkmated king topples over
	// King highlights are set by BoardCoordinator, the camera orbit is handled by MainCameraSystem
	// Effects are cleared by the board synchronization after reset or load
	class GameOverSystem : public PrCore::BaseSystem {
	public:
		GameOverSystem() = default;
		~GameOverSystem() = default;

		void OnCreate() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		void OnGameOver(PrCore::EventPtr p_event);
		void OnBoardChanged(PrCore::EventPtr p_event);

		PrCore::Entity FindKing(Color p_color);

		// Topple animation
		PrCore::Entity     m_toppledKing;
		PrCore::Math::quat m_toppleStartRotation{ 1.0f, 0.0f, 0.0f, 0.0f };
		PrCore::Math::vec3 m_toppleStartPosition{ 0.0f };
		float              m_toppleTime = 0.0f;
	};
}
