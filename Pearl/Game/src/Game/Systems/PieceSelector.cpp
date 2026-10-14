#include "GameConstants.h"
#include "Systems/PieceSelector.h"
#include "Systems/MainCamera.h"
#include "ChessEngine/BoardCoordinator.h"
#include "ChessEngine/ChessEvents.h"


#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/ECS/Components/CoreComponents.h"
#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/Input/InputManager.h"
#include "Engine/Core/Utils/ILogger.h"
#include "Engine/Renderer/Core/IRenderFrontend.h"

namespace ChessGame {

	namespace {
		// Draws the mouse picking ray
		constexpr bool  DrawSelectorDebug = false;
		constexpr float RaycastDistance   = 50.0f;
	}

	void PieceSelectorSystem::OnCreate()
	{
		auto eventManager = PrSystems::Get<PrCore::EventManager>();

		PrCore::EventListener boardChangedListener;
		boardChangedListener.Connect<&PieceSelectorSystem::OnBoardChanged>(this);
		eventManager->AddListener(boardChangedListener, BoardChangedEvent::s_type);

		PrCore::EventListener gameOverListener;
		gameOverListener.Connect<&PieceSelectorSystem::OnBoardChanged>(this);
		eventManager->AddListener(gameOverListener, GameOverEvent::s_type);
	}

	void PieceSelectorSystem::OnEnable()
	{
		m_pMainCamera = m_entityViewer.GetEntityByName(std::string{ MainCameraName });
		m_coordinator = BoardCoordinator::GetInstancePtr();
	}

	void PieceSelectorSystem::OnDisable()
	{
		DeselectPiece();
	}

	void PieceSelectorSystem::OnUpdate(float p_dt)
	{
		if (!m_coordinator)
			return;

		auto input = PrSystems::Get<PrCore::InputManager>();
		auto camera = m_pMainCamera.GetComponent<MainCamera>()->camera;
		auto cameraTransform = m_pMainCamera.GetComponent<PrCore::TransformComponent>();

		if (input->IsKeyPressed(DeselectKey))
			DeselectPiece();

		const bool isClicked = input->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT);
		if (!isClicked && !IsPieceSelected())
			return;

		// One raycast for both click and hover handling
		auto mouseDir = PrCore::Math::normalize(camera->ScreenToWorldSpace(input->GetMousePosition()));
		RaycastHits hits;
		PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(cameraTransform->GetPosition(), mouseDir, RaycastDistance, hits);

		if (isClicked)
			HandleClick(hits);

		if (IsPieceSelected())
			HandleHover(hits);

		if constexpr (DrawSelectorDebug)
			PrSystems::Get<PrRenderer::IRenderFrontend>()->DrawDebugLine(camera->GetPosition() - cameraTransform->GetUpVector() * 0.01f, camera->GetPosition() + mouseDir * RaycastDistance);
	}

	void PieceSelectorSystem::HandleClick(const RaycastHits& p_hits)
	{
		// Entity is copied, GetComponent is not const
		for (auto hit : p_hits)
		{
			// Move the selected piece
			if (IsPieceSelected() && hit.entity.HasComponent<SquareComponent>())
			{
				const Square from = GetSelectedPiece()->square;
				const Square to = hit.entity.GetComponent<SquareComponent>()->square;

				DeselectPiece();
				m_coordinator->TryMovePiece(from, to);
				return;
			}

			// Select a piece
			if (SelectPiece(hit.entity))
			{
				const auto& name = GetSelectedPieceEntity().GetComponent<PrCore::NameComponent>()->name;
				PRLOG_INFO("Selected {} on square {}", name, static_cast<int>(GetSelectedPiece()->square));
				return;
			}
		}
	}

	void PieceSelectorSystem::HandleHover(const RaycastHits& p_hits)
	{
		const Square selectedSquare = GetSelectedPiece()->square;
		const auto& legalMoves = m_coordinator->GetLegalMoves();

		// Lift the piece over the hovered square if it is a legal destination or its own square
		// Entity is copied, GetComponent is not const
		for (auto hit : p_hits)
		{
			if (!hit.entity.HasComponent<SquareComponent>())
				continue;

			const Square hoveredSquare = hit.entity.GetComponent<SquareComponent>()->square;
			bool isValidSquare = hoveredSquare == selectedSquare;
			for (const auto& move : legalMoves)
			{
				if (isValidSquare)
					break;

				isValidSquare = move.fromSquare == selectedSquare && move.toSquare == hoveredSquare;
			}

			if (isValidSquare)
			{
				LiftSelectedPiece(hit.entity.GetComponent<PrCore::TransformComponent>()->GetPosition());
				return;
			}
		}
	}

	bool PieceSelectorSystem::SelectPiece(PrCore::Entity p_overlayEntity)
	{
		if (IsPieceSelected() || !p_overlayEntity.HasComponent<PrCore::ParentComponent>())
			return false;

		PrCore::Entity pieceEntity = p_overlayEntity.GetComponent<PrCore::ParentComponent>()->GetParent();
		if (!pieceEntity.IsValid() || !pieceEntity.HasComponent<PieceComponent>())
			return false;

		PieceComponent* piece = pieceEntity.GetComponent<PieceComponent>();
		if (!m_coordinator->CanSelectPiece(piece->color))
			return false;

		m_selectedOverlay = p_overlayEntity;
		m_selectedOverlay.AddComponent<OverlayComponent>();

		m_coordinator->GenerateMoves();
		m_coordinator->ShowPossibleMovesFor(piece->square);
		LiftSelectedPiece(m_coordinator->GetSquarePos(piece->square));

		return true;
	}

	void PieceSelectorSystem::DeselectPiece()
	{
		if (!IsPieceSelected())
			return;

		// Put the piece back on its square
		if (m_coordinator)
		{
			auto pieceEntity = GetSelectedPieceEntity();
			auto piece = pieceEntity.GetComponent<PieceComponent>();
			if (piece->square != Square::NoSquare)
				pieceEntity.GetComponent<PrCore::TransformComponent>()->SetPosition(m_coordinator->GetSquarePos(piece->square));

			m_coordinator->DisablePossibleMoves();
		}

		m_selectedOverlay.RemoveComponent<OverlayComponent>();
		m_selectedOverlay = PrCore::Entity{};
	}

	bool PieceSelectorSystem::IsPieceSelected() const
	{
		return m_selectedOverlay.IsValid() && !m_selectedOverlay.HasComponent<PrCore::ToDestoryTag>();
	}

	PrCore::Entity PieceSelectorSystem::GetSelectedPieceEntity()
	{
		return m_selectedOverlay.GetComponent<PrCore::ParentComponent>()->GetParent();
	}

	PieceComponent* PieceSelectorSystem::GetSelectedPiece()
	{
		return GetSelectedPieceEntity().GetComponent<PieceComponent>();
	}

	void PieceSelectorSystem::LiftSelectedPiece(PrCore::Math::vec3 p_position)
	{
		p_position.y = PieceLiftHeight;
		GetSelectedPieceEntity().GetComponent<PrCore::TransformComponent>()->SetPosition(p_position);
	}

	void PieceSelectorSystem::OnBoardChanged(PrCore::EventPtr p_event)
	{
		DeselectPiece();
	}
}
