#pragma once
#include "Core/ECS/BaseSystem.h"
#include "Core/ECS/Components.h"
#include "Core/Input/InputManager.h"
#include "Renderer/Core/DeferredRendererFrontend.h"

namespace PrCore::ECS
{
	class TestSystem : public BaseSystem {
	public:

		void OnUpdate(float p_dt) override
		{
			for (auto [entity, transform, _] : m_entityViewer.EntitesWithComponents<TransformComponent, CameraComponent>())
			{
				auto position = transform->GetPosition();
				if (Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::W))
					position.x += p_dt;

				transform->SetPosition(position);
			}
		}

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};

	class RenderStressTest : public BaseSystem {
	public:

		virtual ~RenderStressTest() override
		{
			delete m_cameraTransform;
		}

		void OnEnable() override;
		void OnDisable() override;
		void OnUpdate(float p_dt) override;

		TransformComponent* m_cameraTransform;
		PrRenderer::Core::Camera* m_camera;
		int m_selectedLight = 0;
		int m_maxLight = 0;
		PrRenderer::Resources::LightPtr m_mainLightPtr;

		inline virtual void OnSerialize(Utils::JSON::json& p_serialized) override {}
		inline virtual void OnDeserialize(const Utils::JSON::json& p_deserialized) override {}
	};
}