#pragma once

#include "Renderer/Core/Color.h"
#include "Core/Math/Math.h"

namespace PrRenderer {

	enum class CameraType {
		Perspective,
		Ortographic
	};

	class Camera {
	public:
		Camera();
		Camera(CameraType p_cameraType);

		~Camera() {}

		static Camera* GetMainCamera() { return s_mainCamera; }
		static void SetMainCamera(Camera* p_mainCamera) { s_mainCamera = p_mainCamera; }

		void SetType(CameraType p_type) { m_type = p_type; }
		void SetPosition(const PrCore::Math::vec3& p_position) { m_position = p_position; }
		void SetRotation(const PrCore::Math::vec3& p_rotation) { m_rotation = p_rotation; }
		void SetRotation(const PrCore::Math::quat& p_rotation) { m_rotation = PrCore::Math::eulerAngles(p_rotation); }
		void SetClearColor(const Color& p_clearColor) { m_clearColor = p_clearColor; }
		
		void SetFar(float p_far) { m_far = p_far; }
		void SetNear(float p_near) { m_near = p_near; }
		void SetFOV(float p_FOV) { m_FOV = p_FOV; }
		void SetRatio(float p_ratio) { m_ratio = p_ratio; }
		void SetSize(float p_size) { m_size = p_size; }

		CameraType GetType() const { return m_type; }
		const PrCore::Math::vec3& GetPosition() const { return m_position; }
		const PrCore::Math::vec3& GetRotation() const { return m_rotation; }
		const Color GetClearColor() const { return m_clearColor; }
		
		float GetFar() const { return m_far; }
		float GetNear() const { return m_near; }
		float GetFOV() const { return m_FOV; }
		float GetRatio() const { return m_ratio; }
		float GetSize() const { return m_size; }

		const PrCore::Math::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
		const PrCore::Math::mat4& GetViewMatrix() const { return m_viewMatrix; }
		const PrCore::Math::mat4& GetCameraMatrix() const { return m_cameraMatrix; }

		const PrCore::Math::mat4& RecalculateMatrices();

		PrCore::Math::vec3 ScreenToWorldSpace(PrCore::Math::vec2 p_screenPos);

	private:
		CameraType m_type;

		PrCore::Math::vec3 m_position;
		PrCore::Math::vec3 m_rotation;
		
		Color m_clearColor;

		//Perspective
		float m_far;
		float m_near;
		float m_FOV;
		float m_ratio;
		float m_size;

		PrCore::Math::mat4 m_projectionMatrix;
		PrCore::Math::mat4 m_viewMatrix;
		PrCore::Math::mat4 m_cameraMatrix;

		static Camera* s_mainCamera;
	};
}