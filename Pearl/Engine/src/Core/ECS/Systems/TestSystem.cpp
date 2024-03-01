#include "Core/Common/pearl_pch.h"

#include"Core/ECS/Systems/TestSystem.h"
#include "Renderer/Core/DeferredRendererFrontend.h"

using namespace PrCore::ECS;
using namespace PrRenderer::Core;

void RenderStressTest::OnEnable()
{
	std::srand(std::time(0));
	auto randColor = []()->PrRenderer::Core::Color
	{
		auto color = PrRenderer::Core::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		return color;
	};

	PrRenderer::Core::DefferedRendererFrontend::GetInstance().SetCubemap(nullptr);

	PrRenderer::Core::DefferedRendererFrontend::GetInstance().SetDebugColor(PrRenderer::Core::Color::Red);
	m_camera = PrRenderer::Core::DefferedRendererFrontend::GetInstance().GetCamera();
	m_camera->SetPosition({ 40, 3, 20 });
	m_camera->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 90, 0))));
	auto settings = DefferedRendererFrontend::GetInstance().GetSettingsPtr();
	settings->enableFog = false;

	m_cameraTransform = new TransformComponent();

	m_maxLight = 0;
	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		m_maxLight++;
	}

	//Randomize Lights
	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, MeshRendererComponent>())
	{
		auto light = entity.GetComponent<LightComponent>();
		auto mesh = entity.GetComponent<MeshRendererComponent>();

		mesh->material = std::make_shared<PrRenderer::Resources::Material>(*mesh->material);
		PrRenderer::Core::Color color = randColor();
		light->m_light->SetColor(color);
		mesh->material->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(color));
	}
}

void RenderStressTest::OnDisable()
{
}

void RenderStressTest::OnUpdate(float p_dt)
{
	//Update Camera Transform
	m_cameraTransform->SetPosition(m_camera->GetPosition());
	m_cameraTransform->SetRotation(m_camera->GetRotation());

	auto forward = m_cameraTransform->GetForwardVector();
	auto up = m_cameraTransform->GetUpVector();
	auto right = m_cameraTransform->GetRightVector();

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::L))
		m_selectedLight = (++m_selectedLight) % m_maxLight;

	// Rotate all
	for (auto entity : m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		auto rotate = entity.GetComponent<TransformComponent>()->GetRotation();

		auto name = entity.GetComponent<NameComponent>()->name;
		if (name.find("Ocluder") != std::string::npos ||
			name.find("Light") != std::string::npos ||
			name.find("Emission") != std::string::npos)
		{
			auto rotationQuat = PrCore::Math::quat({ 0.0f, 3.14f / 4.0f * p_dt, 3.14f / 4.0f * p_dt });
			entity.GetComponent<TransformComponent>()->SetRotation(rotate * rotationQuat);
		}
	}

	for (auto entity : m_entityViewer.EntitesWithComponents<TransformComponent, LightComponent, MeshRendererComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();
		auto mesh = entity.GetComponent<MeshRendererComponent>();
		auto box = mesh->mesh->GetBoxVolume();

		//This is selected light
		if (entity.GetComponent<NameComponent>()->name == "Light" + std::to_string(m_selectedLight))
		{
			auto position = transform->GetPosition();
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::UP))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position -= addVec;
			}
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::DOWN))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position += addVec;
			}
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::LEFT))
				position -= right * 20.0f * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::RIGHT))
				position += right * 20.0f * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::PAGE_UP))
				position.y += 20.0f * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::PAGE_DOWN))
				position.y -= 20.0f * p_dt;

			// Change color intensity
			auto color = light->m_light->GetColor();
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_8))
				color += 10.0f * p_dt;
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_2))
				color -= 10.0f * p_dt;

			light->m_light->SetColor(color);

			mesh->material->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(PrCore::Math::max(static_cast<PrCore::Math::vec4>(color), 0.0f)));

			transform->SetPosition(position);
			transform->GenerateWorldMatrix();

			Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), box.GetCenter())
				* Math::scale(Math::mat4(1.0f), box.GetSize() * 1.2f);

			DefferedRendererFrontend::GetInstance().DrawCube(transform->GetWorldMatrix() * transformMat, true);
			if(light->m_shadowCast)
				DefferedRendererFrontend::GetInstance().DrawSphere(position + 0.5f, 0.2f, true);
		}
	}


	// Setup cubemap
	static int cubemap = 2;
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

	// Retup renderer settings
	auto settings = DefferedRendererFrontend::GetInstance().GetSettingsPtr();

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::E))
		settings->enableInstancing = !settings->enableInstancing;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::R))
		settings->enableFog = !settings->enableFog;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::T))
		settings->enableFXAAA = !settings->enableFXAAA;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::Y))
		settings->enableSSAO = !settings->enableSSAO;

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::U))
		settings->enableBloom = !settings->enableBloom;
}