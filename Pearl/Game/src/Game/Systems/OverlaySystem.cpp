#include "Systems/OverlaySystem.h"

#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/Events/ECSEvents.h"

namespace ChessGame {

	namespace {
		constexpr float SelectedMaxAlpha = 0.2f;
		constexpr float SquareAlpha      = 0.2f;

		PrRenderer::Color GetMoveTypeColor(MoveType p_moveType)
		{
			switch (p_moveType)
			{
			case MoveType::Capture:   return PrRenderer::Color{ 1.0f, 0.0f, 0.0f };
			case MoveType::Promotion: return PrRenderer::Color{ 0.0f, 0.0f, 1.0f };
			case MoveType::Castling:  return PrRenderer::Color{ 0.0f, 1.0f, 1.0f };
			case MoveType::EnPassant: return PrRenderer::Color{ 1.0f, 0.0f, 0.5f };
			default:                  return PrRenderer::Color{ 1.0f, 1.0f, 1.0f };
			}
		}
	}

	void OverlaySystem::OnCreate()
	{
		auto eventManager = PrSystems::Get<PrCore::EventManager>();

		PrCore::EventListener overlayAddedListener;
		overlayAddedListener.Connect<&OverlaySystem::OnOverlayAdded>(this);
		eventManager->AddListener(overlayAddedListener, PrCore::ComponentAddedEvent<OverlayComponent>::s_type);

		PrCore::EventListener overlayRemovedListener;
		overlayRemovedListener.Connect<&OverlaySystem::OnOverlayRemoved>(this);
		eventManager->AddListener(overlayRemovedListener, PrCore::ComponentRemovedEvent<OverlayComponent>::s_type);
	}

	void OverlaySystem::OnUpdate(float p_dt)
	{
		// Pulse the selected piece overlay
		for (auto [entity, overlay] : m_entityViewer.EntitesWithComponents<OverlayComponent>())
		{
			if (auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>())
			{
				float alphaValue = SelectedMaxAlpha * (PrCore::Math::sin(overlay->time) + 1.0f) / 2.0f;
				overlay->time += p_dt;

				auto color = renderComponent->mainMaterial->GetColor();
				color.w = alphaValue;

				renderComponent->mainMaterial->SetColor(color);
			}
		}

		// Show possible moves on squares
		for (auto [entity, square] : m_entityViewer.EntitesWithComponents<SquareComponent>())
		{
			if (auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>())
			{
				auto color = GetMoveTypeColor(square->moveType);
				color.w = square->showOverlay ? SquareAlpha : 0.0f;

				renderComponent->mainMaterial->SetColor(color);
			}
		}
	}

	void OverlaySystem::OnOverlayAdded(PrCore::EventPtr p_event)
	{
		// Detach the overlay shape so raycasts go through the selected piece
		auto componentEvent = std::static_pointer_cast<PrCore::ComponentAddedEvent<OverlayComponent>>(p_event);
		auto entity = componentEvent->m_entity;
		if (!entity.HasComponent<PrCore::RigidBodyStaticComponent>())
			return;

		auto rigidBody = entity.GetComponent<PrCore::RigidBodyStaticComponent>()->rigidBody;
		componentEvent->m_component->shapeCache = rigidBody->GetShape();
		rigidBody->DetachShape(componentEvent->m_component->shapeCache);
	}

	void OverlaySystem::OnOverlayRemoved(PrCore::EventPtr p_event)
	{
		auto componentEvent = std::static_pointer_cast<PrCore::ComponentRemovedEvent<OverlayComponent>>(p_event);
		auto entity = componentEvent->m_entity;

		if (entity.HasComponent<PrCore::RigidBodyStaticComponent>() && componentEvent->m_component->shapeCache)
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
}
