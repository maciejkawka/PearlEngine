#pragma once

#include "Systems/MainCamera.h"
#include "ChessEngine/BoardRender.h"
#include "Systems/OverlayComponent.h"

#include "Engine/Core/ECS/BaseSystem.h"

#include "Engine/Renderer/Core/IRenderFrontend.h"
#include "Engine/Renderer/Core/Camera.h"

namespace ChessGame {

	class PieceSelectorSystem : public PrCore::BaseSystem {
	public:
		void OnEnable() override
		{
			m_pMainCamera = m_entityViewer.GetEntityByName("MainCamera");

			m_cor = BoardCoordinator{ &m_entityViewer };
		}

		void OnUpdate(float p_dt) override
		{
			auto camera = m_pMainCamera.GetComponent<MainCamera>()->camera;
			auto cameraTransform = m_pMainCamera.GetComponent<PrCore::TransformComponent>();

			float distance = 50.0f;
			std::vector <PrPhysics::RaycastHit> hits;

			// Deselect always by G
			if (m_selected.IsValid() && PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::G))
			{
				m_selected.RemoveComponent<OverlayComponent>();
				m_render.Deselect();
				m_selected = PrCore::Entity{};
			}

			auto mousePos = camera->ScreenToWorldSpace(PrSystems::Get<PrCore::InputManager>()->GetMousePosition());
			if (PrSystems::Get<PrCore::InputManager>()->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT))
			{
				if (PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(cameraTransform->GetPosition(), PrCore::Math::normalize(mousePos), distance, hits))
				{
					for (auto hit : hits)
					{
						auto name = hit.entity.GetComponent<PrCore::NameComponent>()->name;

						// Move piece
						if (m_selected.IsValid() && name.find("Square") != name.npos)
						{
							auto pos1 = hit.entity.GetComponent<PrCore::TransformComponent>()->GetPosition();
							auto destinationSquare = m_cor.GetSquare(pos1);
							auto piece = m_cor.GetSqureByEntity(m_selected.GetComponent<PrCore::ParentComponent>()->GetParent());

							m_selected.RemoveComponent<OverlayComponent>();
							m_render.Deselect();
							m_selected = PrCore::Entity{};

							// Move piece only if destination square varies
							if (piece != destinationSquare)
							{
								m_cor.MovePiece(piece, destinationSquare);
							}

							break;
						}

						// Select piece
						if (!m_selected.IsValid() && name.find("Overlay") != name.npos && name.find("Square") == name.npos)
						{
							PRLOG_INFO("Selected {} position: x:{} y:{} z:{}", name, hit.position.x, hit.position.y, hit.position.z);

							m_selected = hit.entity;
							m_selected.AddComponent<OverlayComponent>();
							m_render.Select(m_selected);
							break;
						}
					}
				}
			}

			// If selected render hollow on hovered square
			if (m_selected.IsValid())
			{
				std::vector <PrPhysics::RaycastHit> hits;
				if (PrSystems::Get<PrPhysics::PhysicsSystem>()->RaycastAll(cameraTransform->GetPosition(), PrCore::Math::normalize(mousePos), distance, hits))
				{
					for (auto hit : hits)
					{
						auto name = hit.entity.GetComponent<PrCore::NameComponent>()->name;
						if (name.find("Square") != name.npos)
						{
							auto pos = hit.entity.GetComponent<PrCore::TransformComponent>()->GetPosition();
							m_render.ShowMoveHollow(pos);
						}
					}
				}
			}

			PrSystems::Get<PrRenderer::IRenderFrontend>()->DrawDebugLine(camera->GetPosition() - cameraTransform->GetUpVector() * 0.01f, camera->GetPosition() + PrCore::Math::normalize(mousePos) * distance);
		}

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		PrCore::Entity   m_pMainCamera;
		PrCore::Entity   m_selected;
		BoardCoordinator m_cor;
		BoardRender      m_render;
	};
}