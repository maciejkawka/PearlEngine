#pragma once

#include "Engine/Core/ECS/BaseComponent.h"
#include "Engine/Core/ECS/BaseSystem.h"
#include "Engine/Core/Events/Event.h"

#include "Engine/Renderer/Core/Camera.h"

namespace ChessGame {

	class MainCamera : public PrCore::BaseComponent {
	public:
		PrRenderer::Camera* camera = nullptr;
	};

	// Orbit camera around the board, WASD rotates, mouse scroll zooms, number keys switch presets
	// The camera slowly orbits the board after the game ends
	class MainCameraSystem : public PrCore::BaseSystem {
	public:
		MainCameraSystem() = default;
		~MainCameraSystem() = default;

		void OnCreate() override;
		void OnEnable() override;
		void OnUpdate(float p_dt) override;

		void OnSerialize(PrCore::Utils::JSON::json& p_serialized) override {}
		void OnDeserialize(const PrCore::Utils::JSON::json& p_deserialized) override {}

	private:
		struct CameraPreset
		{
			float radius;
			float yaw;
			float rotation;
		};

		void ApplyPreset(const CameraPreset& p_preset);

		void OnGameOver(PrCore::EventPtr p_event);
		void OnBoardChanged(PrCore::EventPtr p_event);

		bool m_isGameOverOrbit = false;

		float m_rotationSpeed = 90.0f;
		float m_rotation = 0.0f;
		float m_yaw = 90.0f;
		float m_radius = 1.0f;

		PrCore::Entity     m_pMainCamera;
		PrCore::Math::vec3 m_lookAt{ -0.0758f, 0.0f, -0.5626f };
	};
}
