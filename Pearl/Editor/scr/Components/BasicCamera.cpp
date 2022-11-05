#include"Core/Utils/Logger.h"

#include"Editor/Components/BasicCamera.h"

#include"Core/Input/InputManager.h"

using namespace PrEditor::Components;
namespace Input = PrCore::Input;

BasicCamera::BasicCamera(PrRenderer::Core::CameraType p_cameraType):
	m_lastMousePos(PrCore::Math::vec2(0.0f)),
	m_rotationSpeed(5.0f),
	m_movementSpeed(3.0f),
	m_fastSpeed(10.0f),
	m_normalSpeed(m_movementSpeed),
	m_camera(nullptr)
{	
	m_camera = new PrRenderer::Core::Camera(p_cameraType);
	m_camera->SetFar(10000.f);
	m_camera->SetPosition({ 0,0,4 });
	PrRenderer::Core::Camera::SetMainCamera(m_camera);
}

BasicCamera::BasicCamera():
	m_lastMousePos(PrCore::Math::vec2(0.0f)),
	m_rotationSpeed(5.0f),
	m_movementSpeed(3.0f),
	m_fastSpeed(10.0f),
	m_normalSpeed(m_movementSpeed),
	m_camera(nullptr)
{
	m_camera = new PrRenderer::Core::Camera();
	m_camera->SetPosition({ 0,0,4 });
	PrRenderer::Core::Camera::SetMainCamera(m_camera);
}

BasicCamera::~BasicCamera()
{
	if(PrRenderer::Core::Camera::GetMainCamera() == m_camera)
		PrRenderer::Core::Camera::SetMainCamera(nullptr);

	delete m_camera;
}

void BasicCamera::Update(float p_dt)
{
	auto position = m_camera->GetPosition();
	auto rotationRadians = m_camera->GetRotation();
	auto rotation = PrCore::Math::degrees(rotationRadians);

	auto deltaMousePos = m_lastMousePos - PrCore::Input::InputManager::GetInstance().GetMousePosition();

	if (Input::InputManager::GetInstance().IsButtonHold(PrCore::Input::PrMouseButton::BUTTON_2))
	{
		rotation.y += m_rotationSpeed * deltaMousePos.x * p_dt;
		rotation.x += m_rotationSpeed * deltaMousePos.y * p_dt;
	}

	if (Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::W))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(0, 0, -1)) * m_movementSpeed * p_dt;
	}
	if (Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::S))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(0, 0, 1)) * m_movementSpeed * p_dt;
	}
	if (Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::A))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(-1, 0, 0)) * m_movementSpeed * p_dt;
	}
	if (Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::D))
	{
		position += PrCore::Math::rotate(PrCore::Math::quat(rotationRadians), PrCore::Math::vec3(1, 0, 0)) * m_movementSpeed * p_dt;
	}

	if (Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::LEFT_SHIFT))
		m_movementSpeed = m_fastSpeed;
	else if(Input::InputManager::GetInstance().IsKeyReleased(Input::PrKey::LEFT_SHIFT))
		m_movementSpeed = m_normalSpeed;

	m_camera->SetPosition(position);
	m_camera->SetRotation(PrCore::Math::radians(rotation));

	m_lastMousePos = Input::InputManager::GetInstance().GetMousePosition();
}
