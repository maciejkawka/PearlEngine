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

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::L))
		lightID = (++lightID) % 5;

	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();

		if (entity.GetComponent<NameComponent>()->name == "MainLight")
		{
			auto rotation = transform->GetLocalEulerRotation();
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_8))
				rotation.x += 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_2))
				rotation.x -= 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_4))
				rotation.y += 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_6))
				rotation.y -= 20 * p_dt;

			transform->SetLocalRotation(Math::quat(Math::radians(rotation)));
		}



		if(entity.GetComponent<NameComponent>()->name == "Light" + std::to_string(lightID + 1))
		{
			auto position = transform->GetPosition();
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::UP))
				position.x += 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::DOWN))
				position.x -= 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::LEFT))
				position.z -= 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::RIGHT))
				position.z += 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::PAGE_UP))
				position.y += 20 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::PAGE_DOWN))
				position.y -= 20 * p_dt;

			transform->SetPosition(position);
			transform->GenerateWorldMatrix();
		}

		DefferedRendererFrontend::GetInstance().AddLight(light, transform, entity.GetID().GetID());
	}

	for(auto entity: m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{

		if(entity.GetComponent<NameComponent>()->name == "Gun")
		{
			auto position = entity.GetComponent<TransformComponent>()->GetPosition();

			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::U))
				position.z += 10 * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::J))
				position.z -= 10 * p_dt;

			entity.GetComponent<TransformComponent>()->SetPosition(position);
		}

		if (entity.GetComponent<NameComponent>()->name == "Light1")
			DefferedRendererFrontend::GetInstance().AddMesh(entity);
			
		DefferedRendererFrontend::GetInstance().AddMesh(entity);
	}
}
