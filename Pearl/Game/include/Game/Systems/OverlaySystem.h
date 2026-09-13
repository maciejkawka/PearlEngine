#pragma once

#include "ChessEngine/ChessSystem.h"
#include "Components/MainComponents.h"

#include "Engine/Core/ECS/BaseSystem.h"

namespace ChessGame {
	class OverlaySystem : public PrCore::BaseSystem {
	public:
		void OnCreate() override
		{
			PrCore::EventListener overlayAddedListener;
			overlayAddedListener.Connect<&OverlaySystem::OnComponentStaticAdded>(this);
			PrSystems::Get<PrCore::EventManager>()->AddListener(overlayAddedListener, PrCore::ComponentAddedEvent<OverlayComponent>::s_type);

			PrCore::EventListener overlayRemovedListener;
			overlayRemovedListener.Connect<&OverlaySystem::OnComponentStaticRemoved>(this);
			PrSystems::Get<PrCore::EventManager>()->AddListener(overlayRemovedListener, PrCore::ComponentRemovedEvent<OverlayComponent>::s_type);
		}

		void OnUpdate(float p_dt) override
		{
			for (auto [entity, overlay] : m_entityViewer.EntitesWithComponents<OverlayComponent>())
			{
				if (auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>())
				{
					float alphaValue = 0.2f * (PrCore::Math::sin(overlay->time) + 1.0f) / 2.0f;
					overlay->time += p_dt;

					auto color = renderComponent->mainMaterial->GetColor();
					color.w = alphaValue;

					renderComponent->mainMaterial->SetColor(color);
				}
			}

			for (auto [entity, overlay] : m_entityViewer.EntitesWithComponents<SquareComponent>())
			{
				if (auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>())
				{
					auto color = renderComponent->mainMaterial->GetColor();
					if (overlay->moveType == MoveType::Capture)
						color = PrRenderer::Color{ 1.0f, 0.0f, 0.0f };
					else if (overlay->moveType == MoveType::Promotion)
						color = PrRenderer::Color{ 0.0f, 0.0f, 1.0f };
					else
						color = PrRenderer::Color{ 1.0f, 1.0f, 1.0f };

					color.w = overlay->showOverlay ? 0.2f : 0.0f;

					renderComponent->mainMaterial->SetColor(color);
				}
			}
		}

	private:
		void OnComponentStaticAdded(PrCore::EventPtr p_eventType)
		{
			auto componentEvent = std::static_pointer_cast<PrCore::ComponentAddedEvent<OverlayComponent>>(p_eventType);
			auto entity = componentEvent->m_entity;
			componentEvent->m_component->shapeCache = entity.GetComponent<PrCore::RigidBodyStaticComponent>()->rigidBody->GetShape();
			entity.GetComponent<PrCore::RigidBodyStaticComponent>()->rigidBody->DetachShape(componentEvent->m_component->shapeCache);
		}

		void OnComponentStaticRemoved(PrCore::EventPtr p_eventType)
		{
			auto componentEvent = std::static_pointer_cast<PrCore::ComponentAddedEvent<OverlayComponent>>(p_eventType);
			auto entity = componentEvent->m_entity;
			if (entity.HasComponent<PrCore::RigidBodyStaticComponent>())
			{
				auto physicsComponent = entity.GetComponent<PrCore::RigidBodyStaticComponent>();
				physicsComponent->rigidBody->AttachShape(componentEvent->m_component->shapeCache);
			}

			if (entity.HasComponent<PrCore::MeshRendererComponent>())
			{
				auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>();
				auto color = renderComponent->mainMaterial->GetColor();
				color.w = 0.0f;
				renderComponent->mainMaterial->SetColor(color);
			}
		}
	};
}