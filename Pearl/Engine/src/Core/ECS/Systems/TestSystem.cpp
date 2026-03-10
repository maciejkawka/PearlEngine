#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/TestSystem.h"
#include "Core/ECS/Components/PhysicsComponents.h"
#include "Core/ECS/Components/SphereBullet.h"
#include "Core/ECS/Components/AudioComponents.h"

#include "Renderer/Core/Color.h"

#include "Physics/Core/Physics.h"
#include "Physics/Shape/IConvexMesh.h"
#include "Physics/Events/Events.h"
#include "Core/File/FileSystem.h"
#include "Renderer/Core/LowRenderer.h"
#include "Renderer/Core/IRenderFrontend.h"
#include "Renderer/Resources/Texture2DLoader.h"

#include "Audio/Core/IAudioSystem.h"
#include "Audio/Core/ISoundEvent.h"
#include "Audio/Core/ISoundBus.h"

using namespace PrCore;
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

	auto entity = m_entityViewer.CreateEntity("CameraCube");
	m_cameraTransform = entity.AddComponent<PrCore::TransformComponent>();
	m_cameraTransform->SetPosition(m_camera->GetPosition());
	m_cameraTransform->SetRotation(m_camera->GetRotation());

	entity.AddComponent<AudioListenerComponent>();

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

			auto elo = entity.GetComponent<PrCore::TransformComponent>()->GetRotation();
			//entity.GetComponent<PrCore::TransformComponent>()->SetRotation(PrCore::Math::inverse(elo));
		}
	}

	pRenderer->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/hrd_skymap.mat").GetData());
	m_mainLightPtr->SetColor(m_lightColor);

	PrSystems::Get<PrPhysics::PhysicsSystem>()->SetGravity(PrCore::Math::vec3{ 0.0f });

	auto pan = m_entityViewer.GetEntityByName("Plane.003");

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

	//PrCore::SceneManager::GetInstance().SaveSceneByReference(PrCore::SceneManager::GetInstance().GetActiveScene(), "scene/deserializeTest.pearl");



	return;

	for (auto [entity, transform, light] : m_entityViewer.EntitesWithComponents<TransformComponent, LightComponent>())
	{
		if (light->mainDirectLight)
			continue;

		auto physcomponent = entity.AddComponent<PrCore::RigidBodyDynamicComponent>();

		PrPhysics::Material material;
		material.restitution = 0.9f;
		auto rigidBody = physcomponent->rigidBody;
		auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::SphereGeometry{ transform->GetScale().x * 0.5f }, material);
		rigidBody->AttachShape(shape);
		rigidBody->SetMass(10.0f);
	}

	// Pan
	//{
	//	PrPhysics::Material material;
	//	auto pan = m_entityViewer.GetEntityByName("Pan");
	//	auto boxVolume = pan.GetComponent<PrCore::MeshRendererComponent>()->mesh->GetBoxVolume();
	//	auto transform = pan.GetComponent<PrCore::TransformComponent>();
	//	auto rigidbodyComponent = pan.AddComponent<PrCore::RigidBodyDynamicComponent>();

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
		auto pan = m_entityViewer.GetEntityByName("Pan");
		auto render = pan.GetComponent<PrCore::MeshRendererComponent>();
		auto transform = pan.GetComponent<PrCore::TransformComponent>();
		auto rigidbody = pan.AddComponent<PrCore::RigidBodyDynamicComponent>()->rigidBody;

		PrPhysics::Material material;
		material.staticFriction = 0.0f;
		material.dynamicFriction = 0.1f;
		material.restitution = .1f;

		auto convexMesh = PrSystems::Get<ResourceSystem>()->Load<PrPhysics::IConvexMesh>("test.phys");

		//auto convexMesh = PrPhysics::PhysicsSystem::GetInstance().CreateConvexMesh(render->mesh);
		//auto resourceHandle = PrSystems::Get<ResourceSystem>()->Register<PrPhysics::IConvexMesh>(convexMesh);
		//PrSystems::Get<ResourceSystem>()->SaveToFile<PrPhysics::IConvexMesh>(resourceHandle.GetID(), "ThisIsTest.phys");

		auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::ConvexGeometry{ convexMesh, transform->GetLocalScale() }, material);
		rigidbody->AttachShape(shape);
	}

	PrSystems::Get<SceneManager>()->SaveSceneByName(PrSystems::Get<SceneManager>()->GetActiveScene()->GetSceneName(), "scene/test_deseriallize.pearl");
}

void RenderStressTest::OnDisable()
{
}

void RenderStressTest::OnUpdate(float p_dt)
{
	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::N))
	{
		PrSystems::Get<PrPhysics::PhysicsSystem>()->SetGravity(PrCore::Math::vec3{ 0.0f });
	}
	else if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::M))
	{
		PrSystems::Get<PrPhysics::PhysicsSystem>()->SetGravity(PrCore::Math::vec3{ 0.0f, -9.81f, 0.0f });
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
		if (PrSystems::Get<PrPhysics::PhysicsSystem>()->Raycast(m_camera->GetPosition(), forward, distance, hit))
		{
			auto name = hit.entity.GetComponent<NameComponent>()->name;
			PRLOG_INFO("Raycast hit entity name {} distance: {}", name, hit.distance);
			PrSystems::Get<IRenderFrontend>()->SetDebugColor(PrRenderer::Color::Green);

			if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::F) && hit.entity.HasComponent<RigidBodyDynamicComponent>())
			{
				auto rigidbody = hit.entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody;
				rigidbody->AddForce(forward * 30.0f, PrPhysics::ForceMode::Impulsive);
			}
		}
		else
			PrSystems::Get<IRenderFrontend>()->SetDebugColor(PrRenderer::Color::Red);
		PrSystems::Get<IRenderFrontend>()->DrawDebugLine(m_camera->GetPosition() - up, m_camera->GetPosition() + forward * distance);
	}

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrKey::L))
		m_selectedLight = (++m_selectedLight) % m_maxLight;


	auto sun = m_entityViewer.GetEntityByName("Sun");
	if (sun.IsValid())
	{
		auto forwardVector = sun.GetComponent<TransformComponent>()->GetForwardVector();
		auto rotation = sun.GetComponent<TransformComponent>()->GetRotation();
		PrSystems::Get<IRenderFrontend>()->DrawDebugSphere(PrCore::Math::vec3{ 0,20,0 }, 1.0f, false);
		PrSystems::Get<IRenderFrontend>()->DrawDebugLine(PrCore::Math::vec3{ 0,20,0 }, PrCore::Math::vec3{ 0,20,0 } + forwardVector * 5.0f);
		if (PrSystems::Get<InputManager>()->IsKeyHold(PrCore::PrKey::K))
		{
			glm::quat deltaRot = glm::angleAxis(glm::radians(20 * p_dt), glm::vec3(1, 0, 0));
			sun.GetComponent<TransformComponent>()->SetRotation(deltaRot * rotation);
		}
	}

	// Setup cubemap
	static int cubemap = 0;
	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::G))
	{
		cubemap++;
		if (cubemap > 2)
			cubemap = 0;

		if (cubemap == 0)
		{
			PrSystems::Get<IRenderFrontend>()->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/hrd_skymap.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);

			//auto entity = m_entityViewer.GetEntityByName("jacaranda_tree_leaves");
			//entity.GetComponent<PrCore::MeshRendererComponent>()->materials[1]->SetColor({ 0.0f, 0.0f, 0.0f, 0.0f });
		}
		else if (cubemap == 1)
		{
			PrSystems::Get<IRenderFrontend>()->SetCubemap(PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/cubemap_default.mat").GetData());
			m_mainLightPtr->SetColor(m_lightColor);

			//auto entity = m_entityViewer.GetEntityByName("jacaranda_tree_leaves");
			//entity.GetComponent<PrCore::MeshRendererComponent>()->materials[1]->SetColor({ 1.0f, 0.6f, 0.0f, 0.0f });
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
		if (i == m_selectedLight)
		{
			//auto box = mesh->mesh->GetBoxVolume();

			auto position = transform->GetPosition();
			auto loko = transform->GetPosition();
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::UP))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position += addVec;
			}
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::DOWN))
			{
				auto addVec = forward * 20.0f * p_dt;
				addVec.y = 0.0f;
				position -= addVec;
			}
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::LEFT))
				position -= right * 20.0f * p_dt;
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::RIGHT))
				position += right * 20.0f * p_dt;
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::PAGE_UP))
				position.y += 20.0f * p_dt;
			if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::PAGE_DOWN))
				position.y -= 20.0f * p_dt;

			// Change color intensity
			//auto color = light->m_light->GetColor();
			//if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::KP_8))
			//	color += 10.0f * p_dt;
			//if (PrSystems::Get<InputManager>()->IsKeyHold(PrKey::KP_2))
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

	for (auto [entity, light] : m_entityViewer.EntitesWithComponents<LightComponent>())
	{
		if (entity.GetComponent<NameComponent>()->name == "PhysicxExplosion")
		{
			entity.Destroy();
		}
	}

	for (auto [entity, bullet, meshRenderer] : m_entityViewer.EntitesWithComponents<SphereBullet, MeshRendererComponent>())
	{
		bullet->time += p_dt;
		if (bullet->time >= bullet->maxTime)
		{
			auto color = PrRenderer::Color::Magenta * (5 * bullet->time);
			meshRenderer->mainMaterial->SetColor(color);
			entity.Destroy();

			//auto audioEvent = entity.GetComponent<PrCore::AudioSourceComponent>();
			//audioEvent->audioEvent->PlayOneShot();

			PrPhysics::Material material;
			material.staticFriction = 0.0f;
			material.dynamicFriction = 0.1f;
			material.restitution = .1f;

			auto explosion = m_entityViewer.CreateEntity("PhysicxExplosion");

			auto transform = explosion.AddComponent<TransformComponent>();
			transform->SetPosition(entity.GetComponent< TransformComponent>()->GetPosition());


			auto light = explosion.AddComponent<LightComponent>()->m_light;
			light->SetColor(Color(1.0f, 0.05f, 0.0f) * 200.0f);
			light->SetType(PrRenderer::LightType::Point);


			auto rigid = explosion.AddComponent<RigidBodyStaticComponent>();
			auto shape = PrSystems::Get<PrPhysics::PhysicsSystem>()->CreateShape(PrPhysics::SphereGeometry{ 5.0f }, material);
			shape->SetFlags(PrPhysics::ShapeFlags::Trigger);
			rigid->rigidBody->AttachShape(shape);
		}
		else if (bullet->time >= (bullet->maxTime - 0.2f))
		{
			auto color = PrRenderer::Color::Magenta * (5 * bullet->time);
			meshRenderer->mainMaterial->SetColor(color);
		}
		else
		{
			auto color = PrRenderer::Color::Red * (5 * bullet->time);
			meshRenderer->mainMaterial->SetColor(color);
			meshRenderer->mainMaterial->SetProperty("emissionInt", (float)std::pow(2, bullet->time));
		}
	}

	if (PrSystems::Get<InputManager>()->IsButtonPressed(PrCore::PrMouseButton::BUTTON_LEFT))
	{
		static int counter = 0;

		auto physicsPtr = PrSystems::Get<PrPhysics::PhysicsSystem>();

		PrPhysics::Material material;
		material.staticFriction = 0.0f;
		material.dynamicFriction = 0.1f;
		material.restitution = .1f;

		auto entity = m_entityViewer.CreateEntity("TNT");
		auto physcomponent = entity.AddComponent<PrCore::RigidBodyDynamicComponent>();

		auto rigidBody = physcomponent->rigidBody;
		auto shape = physicsPtr->CreateShape(PrPhysics::SphereGeometry{ 0.5f }, material);
		rigidBody->AttachShape(shape);

		auto logoTransform = entity.AddComponent<PrCore::TransformComponent>();
		auto logoMesh = entity.AddComponent<PrCore::MeshRendererComponent>();
		logoTransform->SetPosition(m_camera->GetPosition() + m_cameraTransform->GetForwardVector() * 2.0f);
		logoTransform->SetLocalScale(PrCore::Math::vec3(1.0f));
		auto newMaterial = std::make_shared<PrRenderer::Material>(*PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Material>("stress_test/capsule.mat").GetData());
		newMaterial->SetProperty("roughnessValue", 0.9f);
		newMaterial->SetProperty("emissionColor", PrRenderer::Color::Red);
		newMaterial->SetTexture("emissionMap", PrSystems::Get<ResourceSystem>()->Load<PrRenderer::Texture>("texture/checkerboard.png"));
		logoMesh->mainMaterial = newMaterial;
		logoMesh->mesh = PrRenderer::Mesh::CreatePrimitive(PrRenderer::PrimitiveType::Sphere);
		logoMesh->shadowCaster = false;

		if (PrSystems::Get<InputManager>()->IsKeyHold(PrCore::PrKey::LEFT_CONTROL))
		{
			rigidBody->SetLinearVelocity(m_cameraTransform->GetForwardVector() * 50.0f);
		}

		entity.AddComponent<PrCore::SphereBullet>();

		auto audioSource = entity.AddComponent<PrCore::AudioSourceComponent>();
		audioSource->audioEvent = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Explosion");
		audioSource->audioEvent->Play();

		if (++counter % 2 == 0)
		{
			auto lightComponent = entity.AddComponent<LightComponent>();
			auto light = std::make_shared<PrRenderer::Light>();
			light->SetType(PrRenderer::LightType::Point);
			light->SetColor(PrRenderer::Color::Red * 10.0f);
			light->SetRange(60.0f);
			light->SetAttenuation(0.5f, 0.5f);
			lightComponent->m_light = light;
			lightComponent->m_shadowCast = true;
		}
	}

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::F11))
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


	auto pan = m_entityViewer.GetEntityByName("Plane.003");

	if (pan.IsValid())
	{
		static float time = 0;
		time += p_dt;
		auto mat = pan.GetComponent<MeshRendererComponent>()->mainMaterial;
		mat->SetProperty<float>("uTime", time);
	}

	// Retup renderer settings
	auto settings = PrSystems::Get<IRenderFrontend>()->GetSettingsPtr();

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::E))
		settings->enableInstancing = !settings->enableInstancing;

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::R))
		settings->enableFog = !settings->enableFog;

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::T))
		settings->enableFXAAA = !settings->enableFXAAA;

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::Y))
		settings->enableSSAO = !settings->enableSSAO;

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::U))
		settings->enableBloom = !settings->enableBloom;


	// Test Audio
	static bool addAudio = true;
	if (addAudio && m_entityViewer.GetEntityByName("Pan").IsValid())
	{
		auto audioSource = m_entityViewer.GetEntityByName("Pan").AddComponent<AudioSourceComponent>();
		audioSource->audioEvent = PrSystems::Get<PrAudio::IAudioSystem>()->CreateSoundEvent("event:/Music");
		addAudio = false;
	}

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::P))
	{
		if (auto audioSource = m_entityViewer.GetEntityByName("Pan").GetComponent<AudioSourceComponent>())
			audioSource->audioEvent->Play();
	}

	auto audio = PrSystems::Get<PrAudio::IAudioSystem>();
	static auto music = audio->CreateSoundEvent("event:/Music");

	if (m_entityViewer.GetEntityByName("Pan").IsValid())
	{
		auto transformComponent = m_entityViewer.GetEntityByName("Pan").GetComponent<TransformComponent>();
		PrAudio::Attributes3D attributes1;
		attributes1.position = transformComponent->GetPosition();
		attributes1.upVec = transformComponent->GetUpVector();
		attributes1.forwardVec = transformComponent->GetForwardVector();
		music->Set3DAttributes(attributes1);
	}

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::M))
	{
	//	music->SetParameter("Music_Controller", 1.0f);
	//	music->Play();
	}

	if (PrSystems::Get<InputManager>()->IsKeyPressed(PrCore::PrKey::L))
	{
		auto bus = audio->GetSoundBus("bus:/FX");
		if (bus->GetMute())
			bus->SetMute(false);
		else
			bus->SetMute(true);
	}
}

void RenderStressTest::OnCollisionEnter(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionEnter>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::NameComponent>()->name;
    auto nameB = collisionInfo.entityB.GetComponent<PrCore::NameComponent>()->name;

	if (nameA == "TNT" || nameB == "TNT")
		return;

	if (collisionInfo.entityA.HasComponent<PrCore::LightComponent>() && nameB == "Quad")
	{
		collisionInfo.entityA.GetComponent<PrCore::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(PrRenderer::Color::Red) * 15.0f);
		collisionInfo.entityA.GetComponent<PrCore::MeshRendererComponent>()->mainMaterial->SetColor(PrRenderer::Color::Red * 10.0f);
	}

    //PRLOG_INFO("On collision enter, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionExit(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionExit>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::NameComponent>()->name;

	if (nameA == "TNT" || nameB == "TNT")
		return;

	if (collisionInfo.entityA.HasComponent<PrCore::LightComponent>() && nameB == "Quad")
	{
		auto color = PrRenderer::Color(std::rand() % 20, std::rand() % 20, std::rand() % 20, std::rand() % 20);
		collisionInfo.entityA.GetComponent<PrCore::LightComponent>()->m_light->SetColor(static_cast<PrCore::Math::vec4>(color));
		collisionInfo.entityA.GetComponent<PrCore::MeshRendererComponent>()->mainMaterial->SetColor(color);
	}

	//PRLOG_INFO("On collision exit, EntityA: {}, EntityB {}", nameA, nameB);
}

void RenderStressTest::OnCollisionStay(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::CollisionStay>(p_event)->m_collisionInfo;
	auto nameA = collisionInfo.entityA.GetComponent<PrCore::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::NameComponent>()->name;
}

void RenderStressTest::OnTriggerEnter(PrCore::EventPtr p_event)
{
	auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerEnter>(p_event)->m_collisionInfo;

	if (!collisionInfo.entityA.IsValid() || !collisionInfo.entityB.IsValid())
		return;

	auto nameA = collisionInfo.entityA.GetComponent<PrCore::NameComponent>()->name;
	auto nameB = collisionInfo.entityB.GetComponent<PrCore::NameComponent>()->name;
	PRLOG_INFO("On trigger enter, EntityA: {}, EntityB {}", nameA, nameB);


	if (nameA == "PhysicxExplosion" && nameB == "PhysicxExplosion")
		return;

	if (nameA == "PhysicxExplosion" && nameB == "TNT")
		return;

	if (nameA == "PhysicxExplosion" && collisionInfo.entityB.HasComponent<RigidBodyDynamicComponent>())
	{
		auto transformA = collisionInfo.entityA.GetComponent<TransformComponent>();
		auto transformB = collisionInfo.entityB.GetComponent<TransformComponent>();

		auto explisionNormal = transformB->GetPosition() - transformA->GetPosition();
		auto normilized = PrCore::Math::normalize(explisionNormal);

		auto rigidBody = collisionInfo.entityB.GetComponent<RigidBodyDynamicComponent>();
		rigidBody->rigidBody->AddForce(normilized * 50.0f, PrPhysics::ForceMode::Impulsive);
	}

	if (nameB == "PhysicxExplosion" && collisionInfo.entityA.HasComponent<RigidBodyDynamicComponent>())
	{
		auto transformB = collisionInfo.entityB.GetComponent<TransformComponent>();
		auto transformA = collisionInfo.entityA.GetComponent<TransformComponent>();

		auto explisionNormal = transformA->GetPosition() - transformB->GetPosition();

		auto rigidBody = collisionInfo.entityA.GetComponent<RigidBodyDynamicComponent>();
		rigidBody->rigidBody->AddForce(PrCore::Math::vec3{ 0.0f, 10.0f, 0.0f }, PrPhysics::ForceMode::Impulsive);
	}
}

void RenderStressTest::OnTriggerExit(PrCore::EventPtr p_event)
{
	//auto collisionInfo = std::static_pointer_cast<PrPhysics::TriggerExit>(p_event)->m_collisionInfo;
	//auto nameA = collisionInfo.entityA.GetComponent<PrCore::NameComponent>()->name;
	//auto nameB = collisionInfo.entityB.GetComponent<PrCore::NameComponent>()->name;
	//PRLOG_INFO("On trigger exit, EntityA: {}, EntityB {}", nameA, nameB);
}
