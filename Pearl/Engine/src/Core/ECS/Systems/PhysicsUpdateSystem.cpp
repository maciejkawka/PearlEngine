#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/PhysicsUpdateSystem.h"

#include "Core/ECS/Components/PhysicsComponents.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/Input/InputManager.h"

using namespace PrCore::ECS;

PhysicsUpdateSystem::PhysicsUpdateSystem()
{
	m_physics = PrPhysics::PhysicsSystem::GetInstancePtr();
}

void PhysicsUpdateSystem::OnUpdate(float p_dt)
{
	// Update Physics Scene
	{
		for (auto& actor : m_createdActors)
			m_physics->AddActor(actor);
		m_createdActors.clear();

		for (auto& actor : m_removedActors)
			m_physics->AddActor(actor);
		m_createdActors.clear();
	}

	m_entityViewer.MT_EntitesWithComponents<TransformComponent, RigidBodyDynamicComponent>([](Entity entity, auto transform, auto rigidDynamic) {

		auto position = transform->GetPosition();
		auto rotation = transform->GetRotation();

		PrPhysics::Transform physTransform{ rotation, position };
		rigidDynamic->rigidBody->SetGlobalPose(physTransform);
	});

	m_physics->Simulate(p_dt);
	m_physics->FetchResults();

	m_entityViewer.MT_EntitesWithComponents<TransformComponent, RigidBodyDynamicComponent>([](Entity entity, auto transform, auto rigidDynamic) {

		PrPhysics::Transform physTransform = rigidDynamic->rigidBody->GetGlobalPose();
		transform->SetPosition(physTransform.position);
		transform->SetRotation(physTransform.rotation);
	});
}

void PhysicsUpdateSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Custom;

	Events::EventListener dynamicAddedListener;
	dynamicAddedListener.connect<&PhysicsUpdateSystem::OnComponentDynamicCreated>(this);
	Events::EventManager::GetInstance().AddListener(dynamicAddedListener, Events::ComponentAddedEvent<RigidBodyDynamicComponent>::s_type);

	Events::EventListener dynamicRemovedListener;
	dynamicRemovedListener.connect<&PhysicsUpdateSystem::OnComponentDynamicRemoved>(this);
	Events::EventManager::GetInstance().AddListener(dynamicRemovedListener, Events::ComponentRemovedEvent<RigidBodyDynamicComponent>::s_type);

	Events::EventListener staticAddedListener;
	staticAddedListener.connect<&PhysicsUpdateSystem::OnComponentStaticCreated>(this);
	Events::EventManager::GetInstance().AddListener(staticAddedListener, Events::ComponentAddedEvent<RigidBodyStaticComponent>::s_type);

	Events::EventListener staticRemovedListener;
	staticRemovedListener.connect<&PhysicsUpdateSystem::OnComponentStaticRemoved>(this);
	Events::EventManager::GetInstance().AddListener(staticRemovedListener, Events::ComponentRemovedEvent<RigidBodyStaticComponent>::s_type);
}

void PhysicsUpdateSystem::OnComponentDynamicCreated(PrCore::Events::EventPtr p_eventType)
{
	auto componentEvent = std::static_pointer_cast<Events::ComponentAddedEvent<RigidBodyDynamicComponent>>(p_eventType);
	auto component = componentEvent->m_component;

	component->rigidBody = m_physics->CreateRigidDynamic(PrPhysics::Transform{});
	component->rigidBody->SetEntity(componentEvent->m_entity);
	m_createdActors.push_back(component->rigidBody);
}

void PhysicsUpdateSystem::OnComponentDynamicRemoved(PrCore::Events::EventPtr p_eventType)
{
	auto rigidBody = std::static_pointer_cast<Events::ComponentAddedEvent<RigidBodyDynamicComponent>>(p_eventType)->m_component->rigidBody;
	m_removedActors.push_back(rigidBody);
}

void PhysicsUpdateSystem::OnComponentStaticCreated(PrCore::Events::EventPtr p_eventType)
{
	auto componentEvent = std::static_pointer_cast<Events::ComponentAddedEvent<RigidBodyStaticComponent>>(p_eventType);
	auto component = componentEvent->m_component;

	component->rigidBody = m_physics->CreateRigidStatic(PrPhysics::Transform{});
	component->rigidBody->SetEntity(componentEvent->m_entity);
	m_createdActors.push_back(component->rigidBody);
}

void PhysicsUpdateSystem::OnComponentStaticRemoved(PrCore::Events::EventPtr p_eventType)
{
	auto rigidBody = std::static_pointer_cast<Events::ComponentAddedEvent<RigidBodyStaticComponent>>(p_eventType)->m_component->rigidBody;
	m_removedActors.push_back(rigidBody);
}
