#pragma once
#include"Renderer/Core/Defines.h"
#include"Renderer/Core/Color.h"
#include"Core/Math/Math.h"

namespace PrRenderer::Core {

	enum class CameraType {
		Perspective,
		Ortographic
	};

	class Camera {
	public:
		Camera();
		Camera(CameraType p_cameraType);

		~Camera() {}

		inline static Camera* GetMainCamera() { return s_mainCamera; }
		inline static void SetMainCamera(Camera* p_mainCamera) { s_mainCamera = p_mainCamera; }

		inline void SetType(CameraType p_type) { m_type = p_type; }
		inline void SetPosition(const PrCore::Math::vec3& p_position) { m_position = p_position; }
		inline void SetRotation(const PrCore::Math::vec3& p_rotation) { m_rotation = p_rotation; }
		inline void SetRotation(const PrCore::Math::quat& p_rotation) { m_rotation = PrCore::Math::eulerAngles(p_rotation); }
		inline void SetClearColor(const Color& p_clearColor) { m_clearColor = p_clearColor; }
		
		inline void SetFar(float p_far) { m_far = p_far; }
		inline void SetNear(float p_near) { m_near = p_near; }
		inline void SetFOV(float p_FOV) { m_FOV = p_FOV; }
		inline void SetRatio(float p_ratio) { m_ratio = p_ratio; }
		inline void SetSize(float p_size) { m_size = p_size; }

		inline CameraType GetType() { return m_type; }
		inline const PrCore::Math::vec3& GetPosition() { return m_position; }
		inline const PrCore::Math::vec3& GetRotation() { return m_rotation; }
		inline const Color GetClearColor() { return m_clearColor; }
				
		inline float GetFar() { return m_far; }
		inline float GetNear() { return m_near; }
		inline float GetFOV() { return m_FOV; }
		inline float GetRatio() { return m_ratio; }
		inline float GetSize() { return m_size; }

		inline const PrCore::Math::mat4& GetProjectionMatrix() { return m_projectionMatrix; }
		inline const PrCore::Math::mat4& GetViewMatrix() { return m_viewMatrix; }
		inline const PrCore::Math::mat4& GetCameraMatrix() { return m_cameraMatrix; }

		const PrCore::Math::mat4& RecalculateMatrices();

		//Culling in future

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