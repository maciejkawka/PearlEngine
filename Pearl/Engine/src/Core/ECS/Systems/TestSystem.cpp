#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/TestSystem.h"
#include "Core/ECS/Components/PhysicsComponents.h"

#include "Renderer/Core/RenderSystem.h"
#include "Renderer/Core/Color.h"

#include "Physics/Core/Physics.h"
#include "Physics/Shape/IConvexMesh.h"
#include "Physics/Events/Events.h"

using namespace PrCore::ECS;
using namespace PrRenderer::Core;

void RenderStressTest::OnEnable()
{
	{
		Events::EventListener enterListener;
		enterListener.connect<&RenderStressTest::OnCollisionEnter>(this);
		Events::EventManager::GetInstance().AddListener(enterListener, PrPhysics::CollisionEnter::s_type);

		Events::EventListener stayListener;
		stayListener.connect<&RenderStressTest::OnCollisionStay>(this);
		Events::EventManager::GetInstance().AddListener(stayListener, PrPhysics::CollisionStay::s_type);

		Events::EventListener exitListener;
		exitListener.connect<&RenderStressTest::OnCollisionExit>(this);
		Events::EventManager::GetInstance().AddListener(exitListener, PrPhysics::CollisionExit::s_type);
	}

	{
		Events::EventListener enterListener;
		enterListener.connect<&RenderStressTest::OnTriggerEnter>(this);
		Events::EventManager::GetInstance().AddListener(enterListener, PrPhysics::TriggerEnter::s_type);

		Events::EventListener exitListener;
		exitListener.connect<&RenderStressTest::OnTriggerExit>(this);
		Events::EventManager::GetInstance().AddListener(exitListener, PrPhysics::TriggerExit::s_type);
	}

	std::srand(std::time(0));
	auto randColor = []()->PrRenderer::Core::Color
	{
		auto color = PrRenderer::Core::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		return color;
	};

	renderSystem->SetCubemap(nullptr);

	renderSystem->SetDebugColor(PrRenderer::Core::Color::Red);
	m_camera = renderSystem->GetCamera();
	m_camera->SetPosition({ 15, 9, 3 });
	m_camera->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 0))));

	auto scene = PrCore::ECS::SceneManager::GetInstance().GetAllScenes()[0];
	auto entity = scene->CreateEntity("CameraCube");
	m_cameraTransform = entity.AddComponent<PrCore::ECS::TransformComponent>();
	m_cameraTransform->SetPosition(m_camera->GetPosition());
	m_cameraTransform->SetRotation(m_camera->GetRotation());

	auto settings = renderSystem->GetSettingsPtr();
	settings->enableFog = false;

	m_maxLight = 0;
	for (auto [entity, Light] : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		m_maxLight++;
	}

	//Randomize Lights
	for (auto [entity, light, mesh]: m_entityViewer.EntitesWithComponents<LightComponent, MeshRendererComponent>())
	{
		mesh->mainMaterial = std::make_shared<PrRenderer::Resources::Material>(*mesh->mainMaterial.GetData());
		PrRenderer::Core::Color color = randColor();
		light->m_light->SetColor(color);
		mesh->mainMaterial->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(color));
	}

	for (auto [entity, light] : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		if (light->m_light->GetType() == PrRenderer::Resources::LightType::Directional)
		{
			m_mainLightPtr = light->m_light;
			m_lightColor = m_mainLightPtr->GetColor();
			m_mainLightPtr->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });

			auto elo = entity.GetComponent<PrCore::ECS::TransformComponent>()->GetRotation();
			entity.GetComponent<PrCore::ECS::TransformComponent>()->SetRotation(PrCore::Math::inverse(elo));
		}
	}

	renderSystem->SetCubemap(Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("stress_test/hrd_skymap.mat").GetData());
	m_mainLightPtr->SetColor(m_lightColor);

	PrPhysics::PhysicsSystem::GetInstancePtr()->SetGravity(PrCore::Math::vec3{ 0.0f });

	for (auto [entity, transform, light] : m_entityViewer.EntitesWithComponents<TransformComponent, LightComponent>())
	{
		if (light->mainDirectLight)
			continue;

		auto physcomponent = entity.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>();

		PrPhysics::Material material;
		material.restitution = 0.9f;
		auto rigidBody = physcomponent->rigidBody;
		auto shape = PrPhysics::PhysicsSystem::GetInstancePtr()->CreateShape(PrPhysics::SphereGeometry{ transform->GetScale().x * 0.5f }, material);
		rigidBody->AttachShape(shape);
		rigidBody->SetMass(10.0f);
	}

	// Pan
	//{
	//	PrPhysics::Material material;
	//	auto pan = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("Pan");
	//	auto boxVolume = pan.GetComponent<PrCore::ECS::MeshRendererComponent>()->mesh->GetBoxVolume();
	//	auto transform = pan.GetComponent<PrCore::ECS::TransformComponent>();
	//	auto rigidbodyComponent = pan.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>();

	//	PrPhysics::BoxGeometery box;
	//	box.halfExtents = boxVolume.GetExtends() * transform->GetScale();
	//	auto shape = PrPhysics::PhysicsSystem::GetInstancePtr()->CreateShape(box, material);

	//	PrPhysics::Transform trans;
	//	trans.position = boxVolume.GetCenter() * transform->GetLocalScale();
	//	shape->SetLocalPose(trans);

	//	rigidbodyComponent->rigidBody->AttachShape(shape);
	//	rigidbodyComponent->rigidBody->SetMass(2.0f);
	//}

	// Test
	{
		auto pan = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("Pan");
		auto render = pan.GetComponent<PrCore::ECS::MeshRendererComponent>();
		auto transform = pan.GetComponent<PrCore::ECS::TransformComponent>();
		auto rigidbody = pan.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>()->rigidBody;

		PrPhysics::Material material;
		material.staticFriction = 0.0f;
		material.dynamicFriction = 0.1f;
		material.restitution = .1f;

		auto convexMesh = PrCore::Resources::ResourceSystem::GetInstance().Load<PrPhysics::IConvexMesh>("test.phys");

		//auto convexMesh = PrPhysics::PhysicsSystem::GetInstance().CreateConvexMesh(render->mesh);
		//auto resourceHandle = PrCore::Resources::ResourceSystem::GetInstance().Register<PrPhysics::IConvexMesh>(convexMesh);
		//PrCore::Resources::ResourceSystem::GetInstance().SaveToFile<PrPhysics::IConvexMesh>(resourceHandle.GetID(), "ThisIsTest.phys");

		auto shape = PrPhysics::PhysicsSystem::GetInstancePtr()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh.GetData(), transform->GetLocalScale() }, material);
		rigidbody->AttachShape(shape);
	}
}

void RenderStressTest::OnDisable()
{
}

void RenderStressTest::OnUpdate(float p_dt)
{
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::N))
	{
		PrPhysics::PhysicsSystem::GetInstancePtr()->SetGravity(PrCore::Math::vec3{ 0.0f });
	}
	else if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::M))
	{
		PrPhysics::PhysicsSystem::GetInstancePtr()->SetGravity(PrCore::Math::vec3{ 0.0f, -9.81f, 0.0f });
	}

	//Update Camera Transform
	m_cameraTransform->SetPosition(m_camera->GetPosition());
	m_cameraTransform->SetRotation(m_camera->GetRotation());

	auto forward = m_cameraTransform->GetForwardVector();
	auto up = m_cameraTransform->GetUpVector();
	auto right = m_cameraTransform->GetRightVector();
	auto euler = m_cameraTransform->GetEulerRotation();

	// Raycast
	{
		float distance = 50.0f;
		PrPhysics::RaycastHit hit;
		if (PrPhysics::PhysicsSystem::GetInstancePtr()->Raycast(m_camera->GetPosition(), forward, distance, hit))
		{
			auto name = hit.entity.GetComponent<NameComponent>()->name;
			PRLOG_INFO("Raycast hit entity name {} distance: {}", name, hit.distance);
			PrRenderer::Core::renderSystem->SetDebugColor(PrRenderer::Core::Color::Green);

			if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::F) && hit.entity.HasComponent<RigidBodyDynamicComponent>())
			{
				auto rigidbody = hit.entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody;
				rigidbody->AddForce(forward * 30.0f, PrPhysics::ForceMode::Impulsive);
			}
		}
		else
			PrRenderer::Core::renderSystem->SetDebugColor(PrRenderer::Core::Color::Red);
		PrRenderer::Core::renderSystem->DrawDebugLine(m_camera->GetPosition() - up, m_camera->GetPosition() + forward * distance);
	}

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::L))
		m_selectedLight = (++m_selectedLight) % m_maxLight;

	// Setup cubemap
	static int cubemap = 0;
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::G))
	{
		cubemap++;
		if (cubemap > 2)
			cubemap = 0;

		if (cubemap == 0)
		{
			renderSystem->SetCubemap(Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("stress_test/hrd_skymap.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);
		}
		else if (cubemap == 1)
		{
			renderSystem->SetCubemap(Resources::ResourceSystem::GetInstance().Load<PrRenderer::Resources::Material>("stress_test/cubemap_default.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);
		}
		else if (cubemap == 2)
		{
			renderSystem->SetCubemap(nullptr);
			m_mainLightPtr->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
		}
	}

	// Rotate all
	for (auto [entity, mesh, transform] : m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		auto name = entity.GetComponent<NameComponent>()->name;
		if (name.find("Ocluder") != std::string::npos ||
			name.find("Light") != std::string::npos ||
			name.find("Emission") != std::string::npos)
		{
			auto rotate = transform->GetRotation();
			auto rotationQuat = PrCore::Math::quat({ 0.0f, 3.14f / 4.0f * p_dt, 3.14f / 4.0f * p_dt });
			transform->SetRotation(rotate * rotationQuat);
		}
	}

	for (auto [entity, transform, light] : m_entityViewer.EntitesWithComponents<TransformComponent, LightComponent>())
	{
		//This is selected light
		if (entity.GetComponent<NameComponent>()->name == "Light" + std::to_string(m_selectedLight))
		{
			//auto box = mesh->mesh->GetBoxVolume();

			auto position = transform->GetPosition();
			auto loko = transform->GetPosition();
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::UP))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position += addVec;
			}
			if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::DOWN))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position -= addVec;
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

			if (loko != position)
			{
				entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody->SetLinearVelocity(PrCore::Math::vec3{ 0.0f });
				entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody->SetAngularVelocity(PrCore::Math::vec3{ 0.0f });
			}

			light->m_light->SetColor(color);

			//mesh->mainMaterial->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(PrCore::Math::max(static_cast<PrCore::Math::vec4>(color), 0.0f)));

			transform->SetPosition(position);
			transform->GenerateWorldMatrix();

			//Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), box.GetCenter())
				//* Math::scale(Math::mat4(1.0f), box.GetSize() * 1.2f);

			renderSystem->DrawDebugCube(transform->GetWorldMatrix(), true);
			if (light->m_shadowCast)
				renderSystem->DrawDebugSphere(position + 0.5f, 0.2f, true);
		}
	}

	// Retup renderer settings
	auto settings = renderSystem->GetSettingsPtr();

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

void RenderStressTest::OnCollisionEnter(PrCore::Events::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionEnter>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
    auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;

	if (collisionInfo.entityA.HasComponent<PrCore::ECS::LightComponent>() && nameB == "Quad")
	{
		collisionInfo.entityA.GetComponent<PrCore::ECS::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(PrRenderer::Core::Color::Red) * 10.0f);
		collisionInfo.entityA.GetComponent<PrCore::ECS::MeshRendererComponent>()->mainMaterial->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(PrRenderer::Core::Color::Red) * 10.0f);
	}

    //PRLOG_INFO("On collision enter, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionExit(PrCore::Events::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionExit>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;

	if (collisionInfo.entityA.HasComponent<PrCore::ECS::LightComponent>() && nameB == "Quad")
	{
		auto color = PrRenderer::Core::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		collisionInfo.entityA.GetComponent<PrCore::ECS::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(color));
		collisionInfo.entityA.GetComponent<PrCore::ECS::MeshRendererComponent>()->mainMaterial->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(color));
	}

	//PRLOG_INFO("On collision exit, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionStay(PrCore::Events::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionStay>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
}

void RenderStressTest::OnTriggerEnter(PrCore::Events::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerEnter>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
	PRLOG_INFO("On trigger enter, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnTriggerExit(PrCore::Events::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerExit>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
	PRLOG_INFO("On trigger exit, EntityA: {}, EntityB {}", nameA, nameB);
}
