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
			m_radius = 3.0f;
			m_yaw = 89.0f;
			m_rotation = 90.0f;
		}

		void OnUpdate(float p_dt) override
		{
			auto transformComponent = m_pMainCamera.GetComponent<PrCore::TransformComponent>();
			auto cameraComponent = m_pMainCamera.GetComponent<MainCamera>();

			m_radius -= PrSystems::Get<PrCore::InputManager>()->GetMouseScroll() * 0.1f;

			if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::W))
			{
				m_yaw += m_rotationSpeed * p_dt;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::S))
			{
				m_yaw -= m_rotationSpeed * p_dt;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::A))
			{
				m_rotation -= m_rotationSpeed * p_dt;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::D))
			{
				m_rotation += m_rotationSpeed * p_dt;
			}

			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::KEY_1))
			{
				m_radius = 3.0f;
				m_yaw = 89.0f;
				m_rotation = 90.0f;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::KEY_2))
			{
				m_radius = 3.0f;
				m_yaw = 89.0f;
				m_rotation = 270.0f;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::KEY_3))
			{
				m_radius = 3.0f;
				m_yaw = 45.0f;
				m_rotation = 90.0f;
			}
			if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::KEY_4))
			{
				m_radius = 3.0f;
				m_yaw = 45.0f;
				m_rotation = 270.0f;
			}

			m_yaw = PrCore::clamp(m_yaw, 10.0f, 89.0f);
			m_radius = PrCore::clamp(m_radius, 1.0f, 3.0f);
			if (m_rotation >= 360.0f)
				m_rotation -= 360.0f;
			if (m_rotation < 0.0f)
				m_rotation += 360.0f;

			PrCore::vec3 pos;
			float radYaw = PrCore::radians(m_yaw);
			float radRot = PrCore::radians(m_rotation);

			pos.x = m_radius * PrCore::cos(radYaw) * PrCore::sin(radRot);
			pos.y = m_radius * PrCore::sin(radYaw);
			pos.z = m_radius * PrCore::cos(radYaw) * PrCore::cos(radRot);

			auto direction = PrCore::normalize(-pos);
			auto rotation = PrCore::quatLookAt(direction, PrCore::vec3{ 0.0f, 1.0f, 0.0f });

			cameraComponent->camera->SetPosition(m_lookAt + pos);
			cameraComponent->camera->SetRotation(rotation);

			transformComponent->SetPosition(m_lookAt + pos);
			transformComponent->SetRotation(rotation);
		}

	private:
		float m_rotationSpeed = 90.0f;
		float m_rotation = 0.0f;
		float m_yaw = 90.0f;
		float m_radius = 1.0f;

		PrCore::Entity     m_pMainCamera;
		PrCore::Math::vec3 m_lookAt{ -0.0758f, 0.0f, -0.5626f };
	};
}