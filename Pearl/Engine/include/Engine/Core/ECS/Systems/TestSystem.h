#pragma once
#include "Core/ECS/BaseSystem.h"
#include "Core/ECS/Components.h"
#include "Core/Input/InputManager.h"

namespace PrCore::ECS
{
	class TestSystem : public BaseSystem {
	public:

		void OnUpdate(float p_dt) override
		{
			for (auto entity : m_entityViewer.EntitesWithComponents<TransformComponent, CameraComponent>())
			{
				auto transform = entity.GetComponent<TransformComponent>();

				auto position = transform->GetPosition();
				if (PrCore::Input::InputManager::IsKeyHold(Input::PrKey::W))
					position.x += p_dt;

				transform->SetPosition(position);
			}
		}

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};
}