#pragma once

#include "Engine/Core/ECS/BaseComponent.h"
#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/ECS/Components/TransformComponent.h"

#include "Engine/Renderer/Core/Camera.h"

namespace ChessGame {

	class MainCamera : public PrCore::BaseComponent {
	public:
		PrRenderer::Camera* camera;
	};

	class MainCameraSystem : public PrCore::BaseSystem {
	public:
		void OnEnable() override
		{
			m_pMainCamera = m_entityViewer.GetEntityByName("MainCamera");
		}

		void OnUpdate(float p_dt) override
		{
			auto transformComponent = m_pMainCamera.GetComponent<PrCore::TransformComponent>();
			auto cameraComponent = m_pMainCamera.GetComponent<MainCamera>();

			transformComponent->SetPosition(cameraComponent->camera->GetPosition());
			transformComponent->SetRotation(cameraComponent->camera->GetRotation());
		}

	private:
		PrCore::Entity m_pMainCamera;
	};
}