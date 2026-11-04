#pragma once

#include "Components/MainComponents.h"
#include "Systems/PromotionPicker.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"
#include "Engine/Physics/Core/PhysicsSystem.h"

#include <vector>

namespace ChessGame {

	class BoardCoordinator;

	// Selects pieces and moves them with the mouse
	// Click: left click on a piece selects it, left click on a square or an enemy piece moves the selected piece
	// Drag: press on a piece, move the mouse and release over the destination square
	// Moving a pawn to the last rank starts the promotion choice handled by PromotionPicker
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

		// Mouse handling
		void HandlePress(const RaycastHits& p_hits, const PrCore::Math::vec2& p_mousePosition);
		void HandleRelease();
		void HandleHover(const RaycastHits& p_hits);
		void HandlePromotionInput();

		// Drag, the piece flies only over the legal destinations and its own square
		void StartPress(const PrCore::Math::vec2& p_mousePosition);
		void UpdateDrag(const RaycastHits& p_hits, const PrCore::Math::vec2& p_mousePosition);
		void StopDrag();

		// Raycast helpers
		PrCore::Entity FindPieceOverlayHit(const RaycastHits& p_hits) const;
		Square         FindTargetSquare(const RaycastHits& p_hits) const;
		bool           IsSelectablePiece(PrCore::Entity p_overlayEntity) const;

		// Moves
		void MovePiece(Square p_from, Square p_to, Color p_color);
		void MoveSelectedPiece(Square p_to);
		bool IsPromotionMove(Square p_from, Square p_to) const;
		bool IsAllowedSquare(Square p_from, Square p_square) const; // Legal destination or the piece own square

		// Selection
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
		PromotionPicker   m_promotionPicker;

		bool               m_isMouseDown = false;  // Left button pressed on the selected piece
		bool               m_isDragging = false;   // Mouse moved far enough from the press position
		Square             m_dragSquare = Square::NoSquare; // Allowed square the dragged piece flies over
		PrCore::Math::vec2 m_pressMousePosition{ 0.0f };
	};
}
