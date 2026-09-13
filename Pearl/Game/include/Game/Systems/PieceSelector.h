#pragma once

#include "Systems/MainCamera.h"
#include "Components/MainComponents.h"

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Renderer/Core/IRenderFrontend.h"
#include "Engine/Renderer/Core/Camera.h"

namespace ChessGame {
	class PieceSelectorSystem : public PrCore::BaseSystem {
	public:
		void OnEnable() override
		{
			m_pMainCamera = m_entityViewer.GetEntityByName("MainCamera");
			m_coordinator = BoardCoordinator::GetInstancePtr();
		}

		void OnDisable() override 
		{
			DeselectPiece();
		}

		void OnUpdate(float p_dt) override
		{
			const float distance = 50.0f;
			auto camera = m_pMainCamera.GetComponent<MainCamera>()->camera;
			auto cameraTransform = m_pMainCamera.GetComponent<PrCore::TransformComponent>();

			// Deselect always by G
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::G))
			{
				DeselectPiece();
			}

			auto mousePos = camera->ScreenToWorldSpace(PrSystems::Get<PrCore::InputManager>()->GetMousePosition());
			if (PrSystems::Get<PrCore::InputManager>()->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT))
			{
				std::vector <PrPhysics::RaycastHit> hits;
				if (PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(cameraTransform->GetPosition(), PrCore::Math::normalize(mousePos), distance, hits))
				{
					for (auto hit : hits)
					{
						auto parent = hit.entity.GetComponent<PrCore::ParentComponent>();

						// Move piece
						if (m_selectedPiece.IsValid() && hit.entity.HasComponent<SquareComponent>())
						{
							auto pos = hit.entity.GetComponent<PrCore::TransformComponent>()->GetPosition();
							auto destinationSquare = m_coordinator->GetSquare(pos);
							auto squareComponent = hit.entity.GetComponent<SquareComponent>();
							auto pieceComponent = m_selectedPiece.GetComponent<PrCore::ParentComponent>()->GetParent().GetComponent<PieceComponent>();

							DeselectPiece();

							// Move piece only if destination square varies
							if (m_coordinator->TryMovePiece(pieceComponent->square, squareComponent->square))
							{
								m_coordinator->SetBoardDirty();
							}
						}
						// Select piece
						else if (SelectPiece(hit.entity))
						{
							const auto& name = m_selectedPiece.GetComponent<PrCore::NameComponent>()->name;
							PRLOG_INFO("Selected {} position: x:{} y:{} z:{}", name, hit.position.x, hit.position.y, hit.position.z);
							break;
						}
					}
				}
			}

			// If selected render hollow on hovered square
			if (m_selectedPiece.IsValid())
			{
				std::vector <PrPhysics::RaycastHit> hits;
				if (PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(cameraTransform->GetPosition(), PrCore::Math::normalize(mousePos), distance, hits))
				{
					for (auto hit : hits)
					{
						auto name = hit.entity.GetComponent<PrCore::NameComponent>()->name;
						if (name.find("Square") != name.npos)
						{
							LegalMoves moves;
							m_coordinator->GetMoves(moves);

							auto parent = m_selectedPiece.GetComponent<PrCore::ParentComponent>()->GetParent();
							auto squareCompoenent = hit.entity.GetComponent<SquareComponent>();
							for (auto move : moves)
							{
								auto square = parent.GetComponent<PieceComponent>()->square;
								if ((move.fromSquare == square && squareCompoenent->square == move.toSquare) ||
									squareCompoenent->square == square)
								{
									auto pos = hit.entity.GetComponent<PrCore::TransformComponent>()->GetPosition();
									ShowMoveHollow(pos);

									PRLOG_INFO("{} {} {}", pos.x, pos.y, pos.z);
								}
							}
						}
					}
				}
			}

			PrSystems::Get<PrRenderer::IRenderFrontend>()->DrawDebugLine(camera->GetPosition() - cameraTransform->GetUpVector() * 0.01f, camera->GetPosition() + PrCore::Math::normalize(mousePos) * distance);
		}

		bool SelectPiece(PrCore::Entity p_entity)
		{
			PrCore::Entity parentEntity = p_entity.GetComponent<PrCore::ParentComponent>()->GetParent();

			if (!m_selectedPiece.IsValid() && parentEntity.IsValid() && parentEntity.HasComponent<PieceComponent>())
			{
				PieceComponent* piece = parentEntity.GetComponent<PieceComponent>();
				if (piece->color == m_coordinator->SideToMove())
				{
					m_selectedPiece = p_entity;
					m_selectedOriginPos = parentEntity.GetComponent<PrCore::TransformComponent>()->GetPosition();

					OverlayComponent* overlay = m_selectedPiece.AddComponent<OverlayComponent>();
					overlay->pieceComponent = piece;

					m_coordinator->GenerateMoves();
					m_coordinator->ShowPossibleMovesFor(piece->square);
					ShowMoveHollow(m_selectedOriginPos);

					return true;
				}
			}

			return false;
		}

		void DeselectPiece()
		{
			if (m_selectedPiece.IsValid())
			{
				m_selectedPiece.GetComponent<PrCore::ParentComponent>()->GetParent().GetComponent<PrCore::TransformComponent>()->SetPosition(m_selectedOriginPos);
				m_selectedPiece.RemoveComponent<OverlayComponent>();
				m_selectedPiece = PrCore::Entity{};

				m_coordinator->DisablePossibleMoves();
			}
		}

		void ShowMoveHollow(PrCore::Math::vec3 p_pos)
		{
			auto transform = m_selectedPiece.GetComponent<PrCore::ParentComponent>()->GetParent().GetComponent<PrCore::TransformComponent>();
			p_pos.y = 0.1f;
			transform->SetPosition(p_pos);
		}

	private:
		PrCore::Entity      m_pMainCamera;
		PrCore::Entity      m_selectedPiece;
		PrCore::Math::vec3  m_selectedOriginPos;
		
		BoardCoordinator* m_coordinator;
	};
}