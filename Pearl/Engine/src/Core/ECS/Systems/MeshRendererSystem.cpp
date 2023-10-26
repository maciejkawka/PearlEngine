#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Core/DeferredRendererFrontend.h"

using namespace PrCore::ECS;
using namespace PrRenderer::Core;

MeshRendererSystem::~MeshRendererSystem()
{
	delete m_camera;
}

void MeshRendererSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Custom;
	m_camera = new Camera();
	auto cubemapMaterialHDR = Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat");
}

void MeshRendererSystem::OnEnable()
{
}

void MeshRendererSystem::OnDisable()
{
}

void MeshRendererSystem::OnUpdate(float p_dt)
{
	//Set First Camera that was found in the scene
	for (auto entity : m_entityViewer.EntitesWithComponents<CameraComponent, TransformComponent>())
	{
		auto camera = entity.GetComponent<CameraComponent>()->GetCamera();
		auto transform = entity.GetComponent<TransformComponent>();

		camera->SetPosition(transform->GetPosition());
		camera->SetRotation(transform->GetRotation());

		break;
	}

	DefferedRendererFrontend::GetInstance().AddCubemap(Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat"));

	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();

		auto rotation = transform->GetLocalEulerRotation();
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::UP))
			rotation.x += 20 * p_dt;
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::DOWN))
			rotation.x -= 20 * p_dt;
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::LEFT))
			rotation.y += 20 * p_dt;
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::RIGHT))
			rotation.y -= 20 * p_dt;

		transform->SetLocalRotation(Math::quat(Math::radians(rotation)));

		DefferedRendererFrontend::GetInstance().AddLight(light, transform, entity.GetID().GetID());
	}

	for(auto entity: m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		DefferedRendererFrontend::GetInstance().AddMesh(entity);
	}
}
