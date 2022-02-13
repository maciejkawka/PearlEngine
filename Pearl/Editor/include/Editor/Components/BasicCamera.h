#pragma once 
#include"Renderer/Core/Camera.h"


namespace PrEditor::Components {

	class BasicCamera {
	public:
		BasicCamera(PrRenderer::Core::CameraType p_cameraType);
		BasicCamera();

		~BasicCamera();

		void Update(float p_dt);

		inline void SetPosition(const PrCore::Math::vec3& p_position) { m_camera->SetPosition(p_position); }
		inline void SetRotation(const PrCore::Math::vec3& p_rotation) { m_camera->SetRotation(p_rotation); }

		inline const PrCore::Math::vec3& GetPosition() { return m_camera->GetRotation(); }
		inline const PrCore::Math::vec3& GetRotation() { return m_camera->GetRotation(); }

		PrRenderer::Core::Camera* GetCamera() { return m_camera; }

	private:
		PrRenderer::Core::Camera* m_camera;
		PrCore::Math::vec2 m_lastMousePos;
		float m_rotationSpeed;
		float m_movementSpeed;
		float m_fastSpeed;
		float m_normalSpeed;
	};
}