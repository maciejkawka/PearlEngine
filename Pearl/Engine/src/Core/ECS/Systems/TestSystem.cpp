#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/TestSystem.h"
#include "Core/ECS/Components/PhysicsComponents.h"

#include "Renderer/Core/Color.h"

#include "Physics/Core/Physics.h"
#include "Physics/Shape/IConvexMesh.h"
#include "Physics/Events/Events.h"
#include "Core/File/FileSystem.h"
#include "Renderer/Core/LowRenderer.h"
#include "Renderer/Core/IRenderFrontend.h"
#include "Renderer/Resources/Texture2DLoader.h"

using namespace PrCore::ECS;
using namespace PrRenderer;

void RenderStressTest::OnEnable()
{
	{
		EventListener enterListener;
		enterListener.Connect<&RenderStressTest::OnCollisionEnter>(this);
		PrSystems::Get<EventManager>()->AddListener(enterListener, PrPhysics::CollisionEnter::s_type);

		EventListener stayListener;
		stayListener.Connect<&RenderStressTest::OnCollisionStay>(this);
		PrSystems::Get<EventManager>()->AddListener(stayListener, PrPhysics::CollisionStay::s_type);

		EventListener exitListener;
		exitListener.Connect<&RenderStressTest::OnCollisionExit>(this);
		PrSystems::Get<EventManager>()->AddListener(exitListener, PrPhysics::CollisionExit::s_type);
	}

	{
		EventListener enterListener;
		enterListener.Connect<&RenderStressTest::OnTriggerEnter>(this);
		PrSystems::Get<EventManager>()->AddListener(enterListener, PrPhysics::TriggerEnter::s_type);

		EventListener exitListener;
		exitListener.Connect<&RenderStressTest::OnTriggerExit>(this);
		PrSystems::Get<EventManager>()->AddListener(exitListener, PrPhysics::TriggerExit::s_type);
	}

	std::srand(std::time(0));
	auto randColor = []()->PrRenderer::Color
	{
		auto color = PrRenderer::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		return color;
	};

	auto pRenderer = PrSystems::Get<PrRenderer::IRenderFrontend>();
	pRenderer->SetCubemap(nullptr);

	pRenderer->SetDebugColor(PrRenderer::Color::Red);
	m_camera = pRenderer->GetCamera();
	m_camera->SetPosition({ 15, 9, 3 });
	m_camera->SetRotation(PrCore::Math::quat(PrCore::Math::radians(PrCore::Math::vec3(0, 0, 0))));

	auto scene = PrCore::ECS::SceneManager::GetInstance().GetAllScenes()[0];
	auto entity = scene->CreateEntity("CameraCube");
	m_cameraTransform = entity.AddComponent<PrCore::ECS::TransformComponent>();
	m_cameraTransform->SetPosition(m_camera->GetPosition());
	m_cameraTransform->SetRotation(m_camera->GetRotation());

	auto settings = pRenderer->GetSettingsPtr();
	settings->enableFog = false;

	m_maxLight = 0;
	for (auto [entity, Light] : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		m_maxLight++;
	}

	//Randomize Lights
	for (auto [entity, light, mesh] : m_entityViewer.EntitesWithComponents<LightComponent, MeshRendererComponent>())
	{
		entity.GetComponent<NameComponent>()->name;
		mesh->mainMaterial = std::make_shared<PrRenderer::Material>(*mesh->mainMaterial.GetData());
		PrRenderer::Color color = randColor();
		light->m_light->SetColor(color);
		mesh->mainMaterial->SetColor(color);
		mesh->shadowCaster = false;
	}

	for (auto [entity, light] : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		if (light->m_light->GetType() == PrRenderer::LightType::Directional)
		{
			m_mainLightPtr = light->m_light;
			m_lightColor = m_mainLightPtr->GetColor();
			m_mainLightPtr->SetColor({ 0.0f, 0.0f, 0.0f, 1.0f });
			light->mainDirectLight = true;

			auto elo = entity.GetComponent<PrCore::ECS::TransformComponent>()->GetRotation();
			//entity.GetComponent<PrCore::ECS::TransformComponent>()->SetRotation(PrCore::Math::inverse(elo));
		}
	}

	pRenderer->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/hrd_skymap.mat").GetData());
	m_mainLightPtr->SetColor(m_lightColor);

	PrPhysics::PhysicsSystem::GetInstancePtr()->SetGravity(PrCore::Math::vec3{ 0.0f });

	auto pan = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("Plane.003");

	if(pan.IsValid())
	{
		auto shader = PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Shader>("shader/deffered/water.shader");

		auto newMat = PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("water.mat");
		auto mat = pan.GetComponent<MeshRendererComponent>()->mainMaterial;
		//newMat->CopyPropertiesFrom(*mat.GetData());

		newMat->SetProperty<float>("uTime", 0.0f);
		newMat->SetProperty<float>("uAmplitude", 0.6f);
		newMat->SetProperty<float>("uFrequency", 2.0f);
		newMat->SetProperty<float>("uSpeed", 1.5f);
		//newMat->SetTexture("normalMap", PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Texture>("Water_001_NORM.jpg"));
		//newMat->SetTexture("albedoMap", PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Texture>("Water_001_COLOR.jpg"));

		//auto waterMat = 
		pan.GetComponent<MeshRendererComponent>()->mainMaterial = newMat;
		//PrSystems::Get<ResourceSystem>()->SaveToFile<PrRenderer::Mesh>(mesh.GetID(), "test.obj");
		//auto loadedMesh = PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Mesh>("test.obj");
		//pan.GetComponent<MeshRendererComponent>()->mesh = loadedMesh;


		//auto texture = PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Texture>("texture/jacaranda_tree_leaves_diff_4k_New.png");
		//auto data = texture->FetchGPUData();
		//PrSystems::Get<ResourceSystem>()->SaveToFile<PrRenderer::Texture>(texture.GetID(), "test.jpg");
		//pan.GetComponent<MeshRendererComponent>()->materials[1]->SetTexture("albedoMap", texture.GetData());
	}

	//PrCore::ECS::SceneManager::GetInstance().SaveSceneByReference(PrCore::ECS::SceneManager::GetInstance().GetActiveScene(), "scene/deserializeTest.pearl");



	return;

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

		auto convexMesh = PrSystems::Get<ResourceSystem>()->Load<PrPhysics::IConvexMesh>("test.phys");

		//auto convexMesh = PrPhysics::PhysicsSystem::GetInstance().CreateConvexMesh(render->mesh);
		//auto resourceHandle = PrSystems::Get<ResourceSystem>()->Register<PrPhysics::IConvexMesh>(convexMesh);
		//PrSystems::Get<ResourceSystem>()->SaveToFile<PrPhysics::IConvexMesh>(resourceHandle.GetID(), "ThisIsTest.phys");

		auto shape = PrPhysics::PhysicsSystem::GetInstancePtr()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, transform->GetLocalScale() }, material);
		rigidbody->AttachShape(shape);
	}

	PrCore::ECS::SceneManager::GetInstance().SaveSceneByName(PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetSceneName(), "scene/test_deseriallize.pearl");
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
			PrSystems::Get<IRenderFrontend>()->SetDebugColor(PrRenderer::Color::Green);

			if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::F) && hit.entity.HasComponent<RigidBodyDynamicComponent>())
			{
				auto rigidbody = hit.entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody;
				rigidbody->AddForce(forward * 30.0f, PrPhysics::ForceMode::Impulsive);
			}
		}
		else
			PrSystems::Get<IRenderFrontend>()->SetDebugColor(PrRenderer::Color::Red);
		PrSystems::Get<IRenderFrontend>()->DrawDebugLine(m_camera->GetPosition() - up, m_camera->GetPosition() + forward * distance);
	}

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(Input::PrKey::L))
		m_selectedLight = (++m_selectedLight) % m_maxLight;


	auto sun = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("Sun");
	if(sun.IsValid())
	{
		auto forwardVector = sun.GetComponent<TransformComponent>()->GetForwardVector();
		auto rotation = sun.GetComponent<TransformComponent>()->GetRotation();
		PrSystems::Get<IRenderFrontend>()->DrawDebugSphere(PrCore::Math::vec3{ 0,20,0 }, 1.0f, false);
		PrSystems::Get<IRenderFrontend>()->DrawDebugLine(PrCore::Math::vec3{ 0,20,0 }, PrCore::Math::vec3{ 0,20,0 } + forwardVector * 5.0f);
		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::K))
		{
			glm::quat deltaRot = glm::angleAxis(glm::radians(20 * p_dt), glm::vec3(1, 0, 0));
			sun.GetComponent<TransformComponent>()->SetRotation(deltaRot * rotation);
		}
	}

	// Setup cubemap
	static int cubemap = 0;
	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::G))
	{
		cubemap++;
		if (cubemap > 2)
			cubemap = 0;

		if (cubemap == 0)
		{
			PrSystems::Get<IRenderFrontend>()->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/hrd_skymap.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);

			//auto entity = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("jacaranda_tree_leaves");
			//entity.GetComponent<PrCore::ECS::MeshRendererComponent>()->materials[1]->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		else if (cubemap == 1)
		{
			PrSystems::Get<IRenderFrontend>()->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/cubemap_default.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);

			//auto entity = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("jacaranda_tree_leaves");
			//entity.GetComponent<PrCore::ECS::MeshRendererComponent>()->materials[1]->SetColor({ 1.0f, 0.6f, 0.0f, 0.0f });
		}
		else if (cubemap == 2)
		{
			PrSystems::Get<IRenderFrontend>()->SetCubemap(nullptr);
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

	int i = 0;
	for (auto [entity, transform, light] : m_entityViewer.EntitesWithComponents<TransformComponent, LightComponent>())
	{
		//This is selected light
		if (i ==m_selectedLight)
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
			//auto color = light->m_light->GetColor();
			//if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_8))
			//	color += 10.0f * p_dt;
			//if (PrCore::Input::InputManager::GetInstance().IsKeyHold(Input::PrKey::KP_2))
			//	color -= 10.0f * p_dt;

			//if (loko != position)
			//{
			//	if (entity.HasComponent<RigidBodyDynamicComponent>())
			//	{
			//		entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody->SetLinearVelocity(PrCore::Math::vec3{ 0.0f });
			//		entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody->SetAngularVelocity(PrCore::Math::vec3{ 0.0f });
			//	}
			//}

			//light->m_light->SetColor(color);

			//mesh->mainMaterial->SetProperty("albedoValue", static_cast<PrCore::Math::vec4>(PrCore::Math::max(static_cast<PrCore::Math::vec4>(color), 0.0f)));

			transform->SetPosition(position);
			transform->GenerateWorldMatrix();

			//Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), box.GetCenter())
				//* Math::scale(Math::mat4(1.0f), box.GetSize() * 1.2f);

			PrSystems::Get<IRenderFrontend>()->DrawDebugCube(transform->GetWorldMatrix(), true);
			//if (light->m_shadowCast)
				//renderSystem->DrawDebugSphere(position + 0.5f, 0.2f, true);
		}
		i++;
	}

	if (PrCore::Input::InputManager::GetInstance().IsButtonPressed(PrCore::Input::PrMouseButton::BUTTON_LEFT))
	{
		auto physicsPtr = PrPhysics::PhysicsSystem::GetInstancePtr();

		PrPhysics::Material material;
		material.staticFriction = 0.0f;
		material.dynamicFriction = 0.1f;
		material.restitution = .1f;

		auto entity = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->CreateEntity("PhysicsBox");
		auto physcomponent = entity.AddComponent<PrCore::ECS::RigidBodyDynamicComponent>();

		auto rigidBody = physcomponent->rigidBody;
		auto shape = physicsPtr->CreateShape(PrPhysics::SphereGeometry{ 0.5f }, material);
		rigidBody->AttachShape(shape);

		auto logoTransform = entity.AddComponent<PrCore::ECS::TransformComponent>();
		auto logoMesh = entity.AddComponent<PrCore::ECS::MeshRendererComponent>();
		logoTransform->SetPosition(m_camera->GetPosition() + m_cameraTransform->GetForwardVector() * 2.0f);
		logoTransform->SetLocalScale(PrCore::Math::vec3(1.0f));
		logoMesh->mainMaterial = PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/emissionCapsule.mat");
		logoMesh->mesh = PrRenderer::Mesh::CreatePrimitive(PrRenderer::PrimitiveType::Sphere);

		if (PrCore::Input::InputManager::GetInstance().IsKeyHold(PrCore::Input::PrKey::LEFT_CONTROL))
		{
			rigidBody->SetLinearVelocity(m_cameraTransform->GetForwardVector() * 50.0f);
		}
	}

	if (PrCore::Input::InputManager::GetInstance().IsKeyPressed(PrCore::Input::PrKey::F11))
	{
		size_t width, hegiht;
		auto buffer = PrRenderer::LowRenderer::ReadFrontBuffer(width, hegiht);

		auto tex = PrRenderer::Texture2D::Create();
		tex->SetFormat(PrRenderer::TextureFormat::RGB24);
		tex->SetHeight(hegiht);
		tex->SetWidth(width);
		tex->SetData(buffer);
		tex->Apply();

		PrRenderer::Texture2DLoader loader;
		loader.SaveResourceOnDisc(tex, "screen.png");

		delete[] buffer;
	}


	auto pan = PrCore::ECS::SceneManager::GetInstance().GetActiveScene()->GetEntityByName("Plane.003");

	if (pan.IsValid())
	{
		static float time = 0;
		time += p_dt;
		auto mat = pan.GetComponent<MeshRendererComponent>()->mainMaterial;
		mat->SetProperty<float>("uTime", time);
	}

	// Retup renderer settings
	auto settings = PrSystems::Get<IRenderFrontend>()->GetSettingsPtr();

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

void RenderStressTest::OnCollisionEnter(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionEnter>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
    auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;

	if (collisionInfo.entityA.HasComponent<PrCore::ECS::LightComponent>() && nameB == "Quad")
	{
		collisionInfo.entityA.GetComponent<PrCore::ECS::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(PrRenderer::Color::Red) * 15.0f);
		collisionInfo.entityA.GetComponent<PrCore::ECS::MeshRendererComponent>()->mainMaterial->SetColor(PrRenderer::Color::Red * 10.0f);
	}

    //PRLOG_INFO("On collision enter, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionExit(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionExit>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;

	if (collisionInfo.entityA.HasComponent<PrCore::ECS::LightComponent>() && nameB == "Quad")
	{
		auto color = PrRenderer::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		collisionInfo.entityA.GetComponent<PrCore::ECS::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(color));
		collisionInfo.entityA.GetComponent<PrCore::ECS::MeshRendererComponent>()->mainMaterial->SetColor(color);
	}

	//PRLOG_INFO("On collision exit, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionStay(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionStay>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
}

void RenderStressTest::OnTriggerEnter(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerEnter>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
	PRLOG_INFO("On trigger enter, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnTriggerExit(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerExit>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::ECS::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::ECS::NameComponent>()->name;
	PRLOG_INFO("On trigger exit, EntityA: {}, EntityB {}", nameA, nameB);
}
