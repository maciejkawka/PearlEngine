#include "Core/Common/pearl_pch.h"

#include "Core/Windowing/Window.h"

#include "Renderer/Core/Camera.h"

using namespace PrRenderer;
namespace Math = PrCore::Math;

Camera* Camera::s_mainCamera = nullptr;

Camera::Camera() :
	m_FOV(45.0f),
	m_near(1.0f),
	m_far(100.0f),
	m_ratio(16.0f / 9.0f),
	m_size(1.0f),
	m_type(CameraType::Perspective),
	m_projectionMatrix(Math::mat4(1)),
	m_viewMatrix(Math::mat4(1)),
	m_cameraMatrix(Math::mat4(1)),
	m_clearColor(Color::Black),
	m_position(Math::vec3(0)),
	m_rotation(Math::vec3(0))
{}

Camera::Camera(CameraType p_cameraType) :
	m_FOV(45.0f),
	m_near(1.0f),
	m_far(100.0f),
	m_ratio(16.0f/9.0f),
	m_size(1.0f),
	m_type(p_cameraType),
	m_projectionMatrix(Math::mat4(1)),
	m_viewMatrix(Math::mat4(1)),
	m_cameraMatrix(Math::mat4(1)),
	m_clearColor(Color::Black),
	m_position(Math::vec3(0)),
	m_rotation(Math::vec3(0))
{}


const PrCore::Math::mat4& Camera::RecalculateMatrices()
{
	if (m_type == CameraType::Perspective)
		m_projectionMatrix = Math::perspective(m_FOV, m_ratio, m_near, m_far);
	else if (m_type == CameraType::Ortographic)
		m_projectionMatrix = Math::ortho(-m_ratio * m_size, m_ratio * m_size, -m_size, m_size, m_near, m_far);

	auto pitch = Math::rotate(Math::mat4(1), m_rotation.x, Math::vec3(1, 0, 0));
	auto yaw = Math::rotate(Math::mat4(1), m_rotation.y, Math::vec3(0, 1, 0));
	auto roll = Math::rotate(Math::mat4(1), m_rotation.z, Math::vec3(0, 0, 1));

	auto rotationMatrix = yaw * pitch * roll;
	auto translationMatrix = Math::translate(Math::mat4(1), m_position);

	m_viewMatrix = Math::inverse(translationMatrix * rotationMatrix);
	m_cameraMatrix = m_projectionMatrix * m_viewMatrix;

	return m_cameraMatrix;
}

PrCore::Math::vec3 Camera::ScreenToWorldSpace(PrCore::Math::vec2 p_screenPos)
{
	float width = PrSystems::Get<PrCore::IWindow>()->GetWidth();
	float height = PrSystems::Get<PrCore::IWindow>()->GetHeight();

	float x = (2.0f * p_screenPos.x) / width - 1.0f;
	float y = 1.0f - (2.0f * p_screenPos.y) / height;
	float z = 1.0f;

	Math::vec4 retVector{ x, y, z, 1.0f };
	retVector = Math::inverse(m_projectionMatrix) * retVector;
	retVector = Math::inverse(m_viewMatrix) * Math::vec4{ retVector.x, retVector.y, -1.0f, 0.0f };

	return retVector;
}