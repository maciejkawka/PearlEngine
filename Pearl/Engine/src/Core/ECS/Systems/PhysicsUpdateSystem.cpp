#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/PhysicsUpdateSystem.h"

#include "Core/ECS/Components/PhysicsComponents.h"
#include "Core/ECS/Components/TransformComponent.h"
#include "Core/ECS/Components/CoreComponents.h"
#include "Core/Input/InputManager.h"

using namespace PrCore;

PhysicsUpdateSystem::PhysicsUpdateSystem()
{
	m_physics = PrSystems::Get<PrPhysics::PhysicsSystem>();
}

void PhysicsUpdateSystem::OnUpdate(float p_dt)
{
	// Update Physics Scene
	{
		for (auto& actor : m_createdActors)
			m_physics->AddActor(actor);
		m_createdActors.clear();
	}

	m_entityViewer.MT_EntitesWithComponents<TransformComponent, RigidBodyStaticComponent>([](Entity entity, auto transform, auto rigidStatic) {

		if (transform->IsDirty())
		{
			auto position = transform->GetPosition();
			auto rotation = transform->GetRotation();

			PrPhysics::Transform physTransform{ rotation, position };
			rigidStatic->rigidBody->SetGlobalPose(physTransform);
		}
	});

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
	m_updateGroup = (uint8_t)UpdateGroupType::Custom;

	EventListener dynamicAddedListener;
	dynamicAddedListener.Connect<&PhysicsUpdateSystem::OnComponentDynamicCreated>(this);
	PrSystems::Get<EventManager>()->AddListener(dynamicAddedListener, ComponentAddedEvent<RigidBodyDynamicComponent>::s_type);

	EventListener staticAddedListener;
	staticAddedListener.Connect<&PhysicsUpdateSystem::OnComponentStaticCreated>(this);
	PrSystems::Get<EventManager>()->AddListener(staticAddedListener, ComponentAddedEvent<RigidBodyStaticComponent>::s_type);
}

void PhysicsUpdateSystem::OnComponentDynamicCreated(PrCore::EventPtr p_eventType)
{
	auto componentEvent = std::static_pointer_cast<ComponentAddedEvent<RigidBodyDynamicComponent>>(p_eventType);
	auto component = componentEvent->m_component;

	component->rigidBody = m_physics->CreateRigidDynamic(PrPhysics::Transform{});
	component->rigidBody->SetEntity(componentEvent->m_entity);
	m_createdActors.push_back(component->rigidBody);
}

void PhysicsUpdateSystem::OnComponentStaticCreated(PrCore::EventPtr p_eventType)
{
	auto componentEvent = std::static_pointer_cast<ComponentAddedEvent<RigidBodyStaticComponent>>(p_eventType);
	auto component = componentEvent->m_component;

	PrPhysics::Transform physTransform{};
	if (auto transformComponent = componentEvent->m_entity.GetComponent<PrCore::TransformComponent>())
	{
		physTransform.position = transformComponent->GetPosition();
		physTransform.rotation = transformComponent->GetRotation();
	}

	component->rigidBody = m_physics->CreateRigidStatic(physTransform);
	component->rigidBody->SetEntity(componentEvent->m_entity);
	m_createdActors.push_back(component->rigidBody);
}

PhysicsCleanupSyatem::PhysicsCleanupSyatem()
{
	m_physics = PrSystems::Get<PrPhysics::PhysicsSystem>();
}

void PhysicsCleanupSyatem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroupType::Custom;

	EventListener dynamicRemovedListener;
	dynamicRemovedListener.Connect<&PhysicsCleanupSyatem::OnComponentDynamicRemoved>(this);
	PrSystems::Get<EventManager>()->AddListener(dynamicRemovedListener, ComponentRemovedEvent<RigidBodyDynamicComponent>::s_type);

	EventListener staticRemovedListener;
	staticRemovedListener.Connect<&PhysicsCleanupSyatem::OnComponentStaticRemoved>(this);
	PrSystems::Get<EventManager>()->AddListener(staticRemovedListener, ComponentRemovedEvent<RigidBodyStaticComponent>::s_type);
}

void PhysicsCleanupSyatem::OnUpdate(float p_dt)
{
	for (auto& actor : m_removedActors)
	{
		m_physics->RemoveActor(actor);
	}

	m_removedActors.clear();
}

void PhysicsCleanupSyatem::OnComponentStaticRemoved(PrCore::EventPtr p_eventType)
{
	auto rigidBody = std::static_pointer_cast<ComponentAddedEvent<RigidBodyStaticComponent>>(p_eventType)->m_component->rigidBody;
	m_removedActors.push_back(rigidBody);
}

void PhysicsCleanupSyatem::OnComponentDynamicRemoved(PrCore::EventPtr p_eventType)
{
	auto rigidBody = std::static_pointer_cast<ComponentAddedEvent<RigidBodyDynamicComponent>>(p_eventType)->m_component->rigidBody;
	m_removedActors.push_back(rigidBody);
}
