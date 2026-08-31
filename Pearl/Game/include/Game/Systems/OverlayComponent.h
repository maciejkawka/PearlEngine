#pragma once

#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/ECS/Components/RendererComponents.h"
#include "Engine/Core/ECS/Components/PhysicsComponents.h"

#include "Engine/Core/Events/EventManager.h"
#include "Engine/Core/Events/ECSEvents.h"

namespace ChessGame {

	class OverlayComponent : public PrCore::BaseComponent
	{
	public:
		float m_time = 0.0f;
		PrPhysics::IShapePtr m_shapeCache;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {};
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {};
	};

	class PossibleSquareOverlay : public PrCore::BaseComponent
	{

	};

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
					float alphaValue = 0.2f * (PrCore::Math::sin(overlay->m_time) + 1.0f) / 2.0f;
					overlay->m_time += p_dt;

					auto color = renderComponent->mainMaterial->GetColor();
					color.w = alphaValue;

					renderComponent->mainMaterial->SetColor(color);
				}
			}
		}

		void OnComponentStaticAdded(PrCore::EventPtr p_eventType)
		{
			auto componentEvent = std::static_pointer_cast<PrCore::ComponentAddedEvent<OverlayComponent>>(p_eventType);
			auto entity = componentEvent->m_entity;
			componentEvent->m_component->m_shapeCache = entity.GetComponent<PrCore::RigidBodyStaticComponent>()->rigidBody->GetShape();
			entity.GetComponent<PrCore::RigidBodyStaticComponent>()->rigidBody->DetachShape(componentEvent->m_component->m_shapeCache);
		}

		void OnComponentStaticRemoved(PrCore::EventPtr p_eventType)
		{
			auto componentEvent = std::static_pointer_cast<PrCore::ComponentAddedEvent<OverlayComponent>>(p_eventType);
			auto entity = componentEvent->m_entity;
			if (entity.HasComponent<PrCore::RigidBodyStaticComponent>())
			{
				auto physicsComponent = entity.GetComponent<PrCore::RigidBodyStaticComponent>();
				physicsComponent->rigidBody->AttachShape(componentEvent->m_component->m_shapeCache);
			}

			if (entity.HasComponent<PrCore::MeshRendererComponent>())
			{
				auto renderComponent = entity.GetComponent<PrCore::MeshRendererComponent>();
				auto color = renderComponent->mainMaterial->GetColor();
				color.w = 0.0f;
				renderComponent->mainMaterial->SetColor(color);
			}
		}

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {};
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {};
	};
}