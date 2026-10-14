#pragma once

#include "Components/MainComponents.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"
#include "Engine/Physics/Core/PhysicsSystem.h"

#include <vector>

namespace ChessGame {

	class BoardCoordinator;

	// Selects pieces and moves them with the mouse
	// Left click on a piece selects it, left click on a square tries to move the selected piece
	class PieceSelectorSystem : public PrCore::BaseSystem {
	public:
		PieceSelectorSystem() = default;
		~PieceSelectorSystem() = default;

		void OnCreate() override;
		void OnEnable() override;
		void OnDisable() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		using RaycastHits = std::vector<PrPhysics::RaycastHit>;

		void HandleClick(const RaycastHits& p_hits);
		void HandleHover(const RaycastHits& p_hits);

		bool SelectPiece(PrCore::Entity p_overlayEntity);
		void DeselectPiece();
		bool IsPieceSelected() const;

		PrCore::Entity  GetSelectedPieceEntity();
		PieceComponent* GetSelectedPiece();
		void            LiftSelectedPiece(PrCore::Math::vec3 p_position);

		// Selection is cleared when the board is replaced or the game ends
		void OnBoardChanged(PrCore::EventPtr p_event);

		PrCore::Entity    m_pMainCamera;
		PrCore::Entity    m_selectedOverlay; // Overlay child entity of the selected piece
		BoardCoordinator* m_coordinator = nullptr;
	};
}
