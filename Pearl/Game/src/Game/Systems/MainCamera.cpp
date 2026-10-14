#include "GameConstants.h"
#include "Systems/MainCamera.h"

#include "Engine/Core/ECS/Components/TransformComponent.h"
#include "Engine/Core/Input/InputManager.h"

#include <array>
#include <utility>

namespace ChessGame {

	namespace {
		// Limits
		constexpr float MinYaw    = 10.0f;
		constexpr float MaxYaw    = 89.0f;
		constexpr float MinRadius = 1.0f;
		constexpr float MaxRadius = 3.0f;
		constexpr float ScrollZoomSpeed = 0.1f;
	}

	void MainCameraSystem::OnEnable()
	{
		m_pMainCamera = m_entityViewer.GetEntityByName(std::string{ MainCameraName });
		ApplyPreset({ 3.0f, 89.0f, 90.0f });
	}

	void MainCameraSystem::OnUpdate(float p_dt)
	{
		// Presets, top views and angled views from both sides
		static const std::array<std::pair<PrCore::PrKey, CameraPreset>, 4> presets{ {
			{ CameraPreset1Key, { 3.0f, 89.0f,  90.0f } },
			{ CameraPreset2Key, { 3.0f, 89.0f, 270.0f } },
			{ CameraPreset3Key, { 3.0f, 45.0f,  90.0f } },
			{ CameraPreset4Key, { 3.0f, 45.0f, 270.0f } }
		} };

		auto input = PrSystems::Get<PrCore::InputManager>();
		auto transformComponent = m_pMainCamera.GetComponent<PrCore::TransformComponent>();
		auto cameraComponent = m_pMainCamera.GetComponent<MainCamera>();

		m_radius -= static_cast<float>(input->GetMouseScroll()) * ScrollZoomSpeed;

		if (input->IsKeyHold(CameraUpKey))
			m_yaw += m_rotationSpeed * p_dt;
		if (input->IsKeyHold(CameraDownKey))
			m_yaw -= m_rotationSpeed * p_dt;
		if (input->IsKeyHold(CameraLeftKey))
			m_rotation -= m_rotationSpeed * p_dt;
		if (input->IsKeyHold(CameraRightKey))
			m_rotation += m_rotationSpeed * p_dt;

		for (const auto& [key, preset] : presets)
		{
			if (input->IsKeyPressed(key))
				ApplyPreset(preset);
		}

		m_yaw = PrCore::clamp(m_yaw, MinYaw, MaxYaw);
		m_radius = PrCore::clamp(m_radius, MinRadius, MaxRadius);
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

	void MainCameraSystem::ApplyPreset(const CameraPreset& p_preset)
	{
		m_radius = p_preset.radius;
		m_yaw = p_preset.yaw;
		m_rotation = p_preset.rotation;
	}
}
