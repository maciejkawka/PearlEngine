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


}

void MeshRendererSystem::OnEnable()
{
	DefferedRendererFrontend::GetInstance().SetFlag(RendererFlag::CameraPerspectiveRecalculate);
	DefferedRendererFrontend::GetInstance().SetCubemap(Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat"));
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

	static int cubemap = 0;
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::G))
	{
		cubemap++;
		if (cubemap > 2)
			cubemap = 0;

		if (cubemap == 0)
			DefferedRendererFrontend::GetInstance().SetCubemap(Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("skymapHDRMaterial.mat"));
		else if (cubemap == 1)
			DefferedRendererFrontend::GetInstance().SetCubemap(Resources::ResourceLoader::GetInstance().LoadResource<PrRenderer::Resources::Material>("cubemapMaterial.mat"));
		else if (cubemap == 2)
			DefferedRendererFrontend::GetInstance().SetCubemap(nullptr);
	}

	auto settings = DefferedRendererFrontend::GetInstance().GetSettingsPtr();

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::I))
		settings->enableInstancing = !settings->enableInstancing;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::F))
		settings->enableFog = !settings->enableFog;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::R))
		settings->enableFXAAA = !settings->enableFXAAA;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::T))
		settings->enableSSAO = !settings->enableSSAO;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::B))
		settings->enableBloom = !settings->enableBloom;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::L))
		lightID = (++lightID) % 5;


	PrRenderer::Core::Color colors[5] = {

		PrRenderer::Core::Color::White,
		PrRenderer::Core::Color::Magenta,
		PrRenderer::Core::Color::Blue,
		PrRenderer::Core::Color::Green,
		PrRenderer::Core::Color::Red
	};
	int i = 0;
	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent, MeshRendererComponent>())
	{
		auto light = entity.GetComponent<LightComponent>();
		auto mesh = entity.GetComponent<MeshRendererComponent>();

		static float time = 0;
		time += p_dt / 10.0f;
		auto color = light->m_light->GetColor();
		color = colors[i] * (30.0f * (PrCore::Math::abs(PrCore::Math::sin(time + i)))) + 1.5f;
		light->m_light->SetColor(color);
		mesh->material->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(color));
		i++;
	}

	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();
		auto mesh = entity.GetComponent<MeshRendererComponent>();

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
		DefferedRendererFrontend::GetInstance().AddMesh(entity);
	}
}
