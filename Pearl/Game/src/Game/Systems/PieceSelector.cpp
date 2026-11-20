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
#include "Engine/Audio/Core/IAudioSystem.h"
#include "Engine/Audio/Core/ISoundEvent.h"

namespace ChessGame {

	namespace {
		// Draws the mouse picking ray
		constexpr bool  DrawSelectorDebug = false;
		constexpr float RaycastDistance   = 50.0f;

		// Mouse distance in pixels from the press position that starts dragging
		constexpr float DragStartDistance = 5.0f;
	}

	void PieceSelectorSystem::OnMove(PrCore::EventPtr p_event)
	{
		auto moveEvent = std::static_pointer_cast<MoveMadeEvent>(p_event);

		if (moveEvent->m_move.moveType == MoveType::Castling)
		{
			m_castling->Play();
		}
		else if (moveEvent->m_move.moveType == MoveType::Capture)
		{
			m_capture->Play();
		}
		else if (moveEvent->m_move.moveType == MoveType::Promotion)
		{
			m_promotion->Play();
		}
		else
		{
			m_dropEvent->Play();
		}
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

		PrCore::EventListener moveListsner;
		moveListsner.Connect<&PieceSelectorSystem::OnMove>(this);
		eventManager->AddListener(moveListsner, MoveMadeEvent::s_type);

		m_dropEvent = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/PieceDrop");
		m_castling = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Castling");
		m_capture = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Capture");
		m_promotion = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Promotion");
		m_load = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Load");
	}

	void PieceSelectorSystem::OnEnable()
	{
		m_pMainCamera = m_entityViewer.GetEntityByName(std::string{ MainCameraName });
		m_coordinator = BoardCoordinator::GetInstancePtr();
	}

	void PieceSelectorSystem::OnDisable()
	{
		m_promotionPicker.Cancel();
		DeselectPiece();
	}

	void PieceSelectorSystem::OnUpdate(float p_dt)
	{
		if (!m_coordinator)
			return;

		// Promotion choice blocks the piece selection until confirmed or canceled
		if (m_promotionPicker.IsActive())
		{
			HandlePromotionInput();
			return;
		}

		auto input = PrSystems::Get<PrCore::InputManager>();
		auto camera = m_pMainCamera.GetComponent<MainCamera>()->camera;
		auto cameraTransform = m_pMainCamera.GetComponent<PrCore::TransformComponent>();

		if (input->IsKeyPressed(DeselectKey))
			DeselectPiece();

		const bool isPressed = input->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT);
		const bool isReleased = input->IsButtonReleased(PrCore::PrMouseButton::BUTTON_LEFT) ||
			(m_isMouseDown && !isPressed && !input->IsButtonHold(PrCore::PrMouseButton::BUTTON_LEFT));

		if (!isPressed && !isReleased && !IsPieceSelected())
			return;

		// One raycast for press, release, drag and hover handling
		const auto mousePosition = input->GetMousePosition();
		const auto rayOrigin = cameraTransform->GetPosition();
		const auto rayDirection = PrCore::Math::normalize(camera->ScreenToWorldSpace(mousePosition));

		RaycastHits hits;
		PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(rayOrigin, rayDirection, RaycastDistance, hits);

		if (isPressed)
			HandlePress(hits, mousePosition);
		else if (isReleased)
			HandleRelease();

		if (IsPieceSelected())
		{
			if (m_isMouseDown)
				UpdateDrag(hits, mousePosition);
			else
				HandleHover(hits);
		}

		if constexpr (DrawSelectorDebug)
			PrSystems::Get<PrRenderer::IRenderFrontend>()->DrawDebugLine(camera->GetPosition() - cameraTransform->GetUpVector() * 0.01f, camera->GetPosition() + rayDirection * RaycastDistance);
	}

	void PieceSelectorSystem::HandlePress(const RaycastHits& p_hits, const PrCore::Math::vec2& p_mousePosition)
	{
		PrCore::Entity pieceOverlay = FindPieceOverlayHit(p_hits);

		if (IsPieceSelected())
		{
			const Square targetSquare = FindTargetSquare(p_hits);

			// Press on the selected piece, it can be dragged or released to keep the selection
			if (targetSquare == GetSelectedPiece()->square)
			{
				StartPress(p_mousePosition);
				return;
			}

			// Press on another own piece switches the selection
			if (pieceOverlay.IsValid() && IsSelectablePiece(pieceOverlay))
			{
				DeselectPiece();
				if (SelectPiece(pieceOverlay))
					StartPress(p_mousePosition);
				return;
			}

			// Press on a square or an enemy piece moves the selected piece
			if (targetSquare != Square::NoSquare)
				MoveSelectedPiece(targetSquare);

			return;
		}

		if (pieceOverlay.IsValid() && SelectPiece(pieceOverlay))
		{
			const auto& name = GetSelectedPieceEntity().GetComponent<PrCore::NameComponent>()->name;
			PRLOG_INFO("Selected {} on square {}", name, static_cast<int>(GetSelectedPiece()->square));

			StartPress(p_mousePosition);
		}
	}

	void PieceSelectorSystem::HandleRelease()
	{
		const bool wasDragging = m_isDragging;
		const Square dropSquare = m_dragSquare;
		StopDrag();

		// Release without dragging keeps the selection for the click to move
		if (!wasDragging || !IsPieceSelected())
			return;

		// The piece flies only over allowed squares, dropping it over own square keeps the selection
		const Square selectedSquare = GetSelectedPiece()->square;
		if (dropSquare == Square::NoSquare || dropSquare == selectedSquare)
		{
			DeselectPiece();
			return;
		}

		MoveSelectedPiece(dropSquare);
	}

	void PieceSelectorSystem::HandleHover(const RaycastHits& p_hits)
	{
		// Lift the piece over the hovered square if it is allowed, otherwise keep the last position
		const Square hoveredSquare = FindTargetSquare(p_hits);
		if (IsAllowedSquare(GetSelectedPiece()->square, hoveredSquare))
			LiftSelectedPiece(m_coordinator->GetSquarePos(hoveredSquare));
	}

	void PieceSelectorSystem::HandlePromotionInput()
	{
		auto input = PrSystems::Get<PrCore::InputManager>();

		if (input->IsKeyPressed(PromotionCancelKey) || input->IsKeyPressed(DeselectKey) ||
			input->IsButtonPressed(PrCore::PrMouseButton::BUTTON_RIGHT))
		{
			PRLOG_INFO("Promotion canceled");
			m_promotionPicker.Cancel();
			return;
		}

		if (input->IsKeyPressed(PromotionNextKey) || input->IsKeyPressed(PromotionNextAltKey))
			m_promotionPicker.Next();

		if (input->IsKeyPressed(PromotionPreviousKey))
			m_promotionPicker.Previous();

		if (input->IsKeyPressed(PromotionConfirmKey) || input->IsKeyPressed(PrCore::PrKey::KP_ENTER) ||
			input->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT))
		{
			const Square from = m_promotionPicker.GetFromSquare();
			const Square to = m_promotionPicker.GetToSquare();
			const PieceType choice = m_promotionPicker.GetChoice();

			m_promotionPicker.Cancel();
			m_coordinator->TryMovePiece(from, to, choice);
		}
	}

	void PieceSelectorSystem::StartPress(const PrCore::Math::vec2& p_mousePosition)
	{
		m_isMouseDown = true;
		m_isDragging = false;
		m_dragSquare = GetSelectedPiece()->square;
		m_pressMousePosition = p_mousePosition;
	}

	void PieceSelectorSystem::UpdateDrag(const RaycastHits& p_hits, const PrCore::Math::vec2& p_mousePosition)
	{
		m_isDragging = true;

		// Fly over the hovered square only if it is allowed, otherwise go back over own square
		const Square selectedSquare = GetSelectedPiece()->square;
		const Square hoveredSquare = FindTargetSquare(p_hits);
		const bool isAllowed = IsAllowedSquare(selectedSquare, hoveredSquare);
		m_dragSquare = isAllowed ? hoveredSquare : selectedSquare;
		
		if (isAllowed)
		{
			m_dragSquare = hoveredSquare;
			LiftSelectedPiece(m_coordinator->GetSquarePos(m_dragSquare));
		}
	}

	void PieceSelectorSystem::StopDrag()
	{
		m_isMouseDown = false;
		m_isDragging = false;
		m_dragSquare = Square::NoSquare;
	}

	PrCore::Entity PieceSelectorSystem::FindPieceOverlayHit(const RaycastHits& p_hits) const
	{
		// Entity is copied, GetComponent is not const
		for (auto hit : p_hits)
		{
			if (!hit.entity.HasComponent<PrCore::ParentComponent>())
				continue;

			PrCore::Entity parent = hit.entity.GetComponent<PrCore::ParentComponent>()->GetParent();
			if (parent.IsValid() && parent.HasComponent<PieceComponent>())
				return hit.entity;
		}

		return PrCore::Entity{};
	}

	Square PieceSelectorSystem::FindTargetSquare(const RaycastHits& p_hits) const
	{
		// A piece hit is more accurate than the square behind it when the ray goes through the piece top
		PrCore::Entity pieceOverlay = FindPieceOverlayHit(p_hits);
		if (pieceOverlay.IsValid())
			return pieceOverlay.GetComponent<PrCore::ParentComponent>()->GetParent().GetComponent<PieceComponent>()->square;

		// Entity is copied, GetComponent is not const
		for (auto hit : p_hits)
		{
			if (hit.entity.HasComponent<SquareComponent>())
				return hit.entity.GetComponent<SquareComponent>()->square;
		}

		return Square::NoSquare;
	}

	bool PieceSelectorSystem::IsSelectablePiece(PrCore::Entity p_overlayEntity) const
	{
		PrCore::Entity pieceEntity = p_overlayEntity.GetComponent<PrCore::ParentComponent>()->GetParent();
		return m_coordinator->CanSelectPiece(pieceEntity.GetComponent<PieceComponent>()->color);
	}

	void PieceSelectorSystem::MovePiece(Square p_from, Square p_to, Color p_color)
	{
		// Promotion waits for the player choice, the move is made after the confirmation
		if (IsPromotionMove(p_from, p_to))
		{
			auto previewPosition = m_coordinator->GetSquarePos(p_to);
			previewPosition.y = PieceLiftHeight;
			m_promotionPicker.Start(p_from, p_to, p_color, previewPosition);
			return;
		}

		m_coordinator->TryMovePiece(p_from, p_to);
	}

	void PieceSelectorSystem::MoveSelectedPiece(Square p_to)
	{
		const Square from = GetSelectedPiece()->square;
		const Color color = GetSelectedPiece()->color;

		DeselectPiece();
		MovePiece(from, p_to, color);
	}

	bool PieceSelectorSystem::IsPromotionMove(Square p_from, Square p_to) const
	{
		for (const auto& move : m_coordinator->GetLegalMoves())
		{
			if (move.fromSquare == p_from && move.toSquare == p_to && move.moveType == MoveType::Promotion)
				return true;
		}

		return false;
	}

	bool PieceSelectorSystem::IsAllowedSquare(Square p_from, Square p_square) const
	{
		if (p_square == Square::NoSquare)
			return false;

		if (p_square == p_from)
			return true;

		for (const auto& move : m_coordinator->GetLegalMoves())
		{
			if (move.fromSquare == p_from && move.toSquare == p_square)
				return true;
		}

		return false;
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
		StopDrag();

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
		if(m_load)
			m_load->Play();
		m_promotionPicker.Cancel();
		DeselectPiece();
	}
}
