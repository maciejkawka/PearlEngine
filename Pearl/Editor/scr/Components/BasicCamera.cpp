#include "Editor/Components/BasicCamera.h"

#include "Core/Input/InputManager.h"
#include "Core/Utils/Logger.h"

using namespace PrEditor::Components;

BasicCamera::BasicCamera(PrRenderer::CameraType p_cameraType):
	m_lastMousePos(PrCore::Math::vec2(0.0f)),
	m_rotationSpeed(5.0f),
	m_movementSpeed(3.0f),
	m_fastSpeed(10.0f),
	m_normalSpeed(m_movementSpeed),
	m_camera(nullptr)
{	
	m_camera = new PrRenderer::Camera(p_cameraType);
	m_camera->SetFar(200.0f);
	m_camera->SetPosition({ 0,0,4 });
	PrRenderer::Camera::SetMainCamera(m_camera);
}

BasicCamera::BasicCamera():
	m_lastMousePos(PrCore::Math::vec2(0.0f)),
	m_rotationSpeed(5.0f),
	m_movementSpeed(3.0f),
	m_fastSpeed(10.0f),
	m_normalSpeed(m_movementSpeed),
	m_camera(nullptr)
{
	m_camera = new PrRenderer::Camera();
	m_camera->SetPosition({ 0,0,4 });
	PrRenderer::Camera::SetMainCamera(m_camera);
}

BasicCamera::~BasicCamera()
{
	if(PrRenderer::Camera::GetMainCamera() == m_camera)
		PrRenderer::Camera::SetMainCamera(nullptr);

	delete m_camera;
}

void BasicCamera::Update(float p_dt)
{
	auto position = m_camera->GetPosition();
	auto rotationRadians = m_camera->GetRotation();
	auto rotation = PrCore::Math::degrees(rotationRadians);

	auto deltaMousePos = m_lastMousePos - PrSystems::Get<PrCore::InputManager>()->GetMousePosition();

	if (PrSystems::Get<PrCore::InputManager>()->IsButtonHold(PrCore::PrMouseButton::BUTTON_2))
	{
		rotation.y += m_rotationSpeed * deltaMousePos.x * p_dt;
		rotation.x += m_rotationSpeed * deltaMousePos.y * p_dt;
	}

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::W))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(0, 0, -1)) * m_movementSpeed * p_dt;
	}
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::S))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(0, 0, 1)) * m_movementSpeed * p_dt;
	}
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::A))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(-1, 0, 0)) * m_movementSpeed * p_dt;
	}
	if (PrSystems::Get<PrCore::InputManager>()->IsKeyHold(PrCore::PrKey::D))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(1, 0, 0)) * m_movementSpeed * p_dt;
	}

	if (PrSystems::Get<PrCore::InputManager>()->IsKeyPressed(PrCore::PrKey::LEFT_SHIFT))
		m_movementSpeed = m_fastSpeed;
	else if(PrSystems::Get<PrCore::InputManager>()->IsKeyReleased(PrCore::PrKey::LEFT_SHIFT))
		m_movementSpeed = m_normalSpeed;

	m_camera->SetPosition(position);
	m_camera->SetRotation(PrCore::Math::radians(rotation));

	m_lastMousePos = PrSystems::Get<PrCore::InputManager>()->GetMousePosition();
}
