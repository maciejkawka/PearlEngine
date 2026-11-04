#include "Systems/OverlaySystem.h"

#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/Events/ECSEvents.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"

namespace ChessGame {

	namespace {
		constexpr float SelectedMaxAlpha = 0.2f;
		constexpr float SquareAlpha      = 0.2f;
		constexpr float LastMoveAlpha    = 0.25f;
		constexpr float LastMoveFadeTime = 5.0f;  // Seconds until the last move highlight disappears
		constexpr float CheckMinAlpha    = 0.1f;
		constexpr float CheckMaxAlpha    = 0.6f;
		constexpr float CheckFlashSpeed  = 6.0f;

		constexpr float GameOverMinAlpha = 0.2f;
		constexpr float GameOverMaxAlpha = 0.5f;
		constexpr float GameOverPulseSpeed = 2.0f;

		const PrRenderer::Color SelectedColor{ 1.0f, 1.0f, 0.0f }; // Matches overlay.mat albedo
		const PrRenderer::Color LastMoveColor{ 1.0f, 0.85f, 0.2f };
		const PrRenderer::Color CheckColor{ 1.0f, 0.0f, 0.0f };
		const PrRenderer::Color WinnerColor{ 1.0f, 0.75f, 0.1f };
		const PrRenderer::Color DrawColor{ 0.4f, 0.7f, 1.0f };

		void SetOverlayColor(PrCore::MeshRendererComponent* p_renderComponent, PrRenderer::Color p_color, float p_alpha)
		{
			p_color.w = p_alpha;
			p_renderComponent->mainMaterial->SetColor(p_color);
		}

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
		m_time += p_dt;
		const float checkFlash = (PrCore::Math::sin(m_time * CheckFlashSpeed) + 1.0f) / 2.0f;
		const float checkAlpha = CheckMinAlpha + (CheckMaxAlpha - CheckMinAlpha) * checkFlash;

		const float gameOverPulse = 1.0f;
		const float gameOverAlpha = GameOverMinAlpha + (GameOverMaxAlpha - GameOverMinAlpha) * gameOverPulse;

		// Piece overlays are children of the piece entities
		for (auto [entity, parentComponent, renderComponent] : m_entityViewer.EntitesWithComponents<PrCore::ParentComponent, PrCore::MeshRendererComponent>())
		{
			PrCore::Entity pieceEntity = parentComponent->GetParent();
			if (!pieceEntity.IsValid() || !pieceEntity.HasComponent<PieceComponent>())
				continue;

			if (entity.HasComponent<OverlayComponent>())
			{
				// Pulse the selected piece
				auto overlay = entity.GetComponent<OverlayComponent>();
				float alphaValue = SelectedMaxAlpha * (PrCore::Math::sin(overlay->time) + 1.0f) / 2.0f;
				overlay->time += p_dt;

				SetOverlayColor(renderComponent, SelectedColor, alphaValue);
			}
			else
			{
				switch (pieceEntity.GetComponent<PieceComponent>()->highlight)
				{
				case PieceHighlight::Check:  SetOverlayColor(renderComponent, CheckColor, checkAlpha);       break;
				case PieceHighlight::Winner: SetOverlayColor(renderComponent, WinnerColor, gameOverAlpha);   break;
				case PieceHighlight::Draw:   SetOverlayColor(renderComponent, DrawColor, gameOverAlpha);     break;
				default:                     SetOverlayColor(renderComponent, SelectedColor, 0.0f);          break;
				}
			}
		}

		// Square overlays, the last move highlight fades out
		for (auto [entity, square] : m_entityViewer.EntitesWithComponents<SquareComponent>())
		{
			if (auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>())
			{
				if (square->isLastMove)
					square->lastMoveTime += p_dt;

				if (square->showOverlay)
				{
					SetOverlayColor(renderComponent, GetMoveTypeColor(square->moveType), SquareAlpha);
				}
				else if (square->isLastMove && square->lastMoveTime < LastMoveFadeTime)
				{
					const float fade = 1.0f - square->lastMoveTime / LastMoveFadeTime;
					SetOverlayColor(renderComponent, LastMoveColor, LastMoveAlpha * fade);
				}
				else
				{
					SetOverlayColor(renderComponent, LastMoveColor, 0.0f);
				}
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
