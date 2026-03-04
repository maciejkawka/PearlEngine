#include "Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/AudioSystem.h"

#include "Core/ECS/Components/TransformComponent.h"
#include "Core/ECS/Components/AudioComponents.h"
#include "Audio/Core/IAudioSystem.h"

using namespace PrCore;

void AudioSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroupType::Custom;

	EventListener listenerAdded;
	listenerAdded.Connect<&AudioSystem::OnListenerEnabled>(this);
	PrSystems::Get<EventManager>()->AddListener(listenerAdded, ComponentAddedEvent<AudioListenerComponent>::s_type);
}

void AudioSystem::OnUpdate(float p_dt)
{
	m_entityViewer.MT_EntitesWithComponents<TransformComponent, AudioSourceComponent>([](Entity entity, TransformComponent* transform, AudioSourceComponent* audioSource) {
		if (transform->IsDirty() && audioSource->audioEvent && audioSource->audioEvent->Is3D())
		{
			auto velocity = entity.HasComponent<RigidBodyDynamicComponent>() ?
				entity.GetComponent<RigidBodyDynamicComponent>()->rigidBody->GetLinearVelocity() : PrCore::Math::vec3{ 0.0f };

			PrAudio::Attributes3D attributes{
				transform->GetPosition(),
				velocity,
				transform->GetForwardVector(),
				transform->GetUpVector()
			};

			audioSource->audioEvent->Set3DAttributes(attributes);
		}
		});

	if (m_activeListenerTransform)
	{
		auto velocity = m_activeListenerRigibBody ? m_activeListenerRigibBody->rigidBody->GetLinearVelocity() : PrCore::Math::vec3{ 0.0f };

		PrAudio::Attributes3D attributes{
		m_activeListenerTransform->GetPosition(),
		velocity,
		m_activeListenerTransform->GetForwardVector(),
		m_activeListenerTransform->GetUpVector()
		};

		PrSystems::Get<PrAudio::IAudioSystem>()->SetListenerAttributes(attributes);
	}

	PrSystems::Get<PrAudio::IAudioSystem>()->Update();
}

void AudioSystem::OnListenerEnabled(PrCore::EventPtr p_event)
{
	auto eventListener = std::static_pointer_cast<ComponentAddedEvent<AudioListenerComponent>>(p_event);
	auto entity = eventListener->m_entity;

	if (auto transformComponent = entity.GetComponent<TransformComponent>())
	{
		m_activeListenerTransform = transformComponent;
		m_activeListenerRigibBody = entity.HasComponent<RigidBodyDynamicComponent>() ? entity.GetComponent<RigidBodyDynamicComponent>() : nullptr;


	}
	else
	{
		m_activeListenerTransform = nullptr;
	}
}