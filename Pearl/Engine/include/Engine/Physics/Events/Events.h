#pragma once

#include "Physics/Actor/IRigidBody.h"

#include "Core/Events/Event.h"
#include "Core/ECS/EntityManager.h"

namespace PrPhysics {

	struct CollisionInfo
	{
		PrCore::ECS::Entity entityA;
		PrCore::ECS::Entity entityB;
	};

	class CollisionEnter : public PrCore::Event {
	public:
		CollisionEnter(const CollisionInfo& p_collisionInfo) :
			m_collisionInfo(p_collisionInfo)
		{}

		CollisionInfo m_collisionInfo;

		DEFINE_EVENT_GUID(0xc8b49d55);
	};

	class CollisionExit : public PrCore::Event {
	public:
		CollisionExit(const CollisionInfo& p_collisionInfo) :
			m_collisionInfo(p_collisionInfo)
		{}

		CollisionInfo m_collisionInfo;

		DEFINE_EVENT_GUID(0xc006569a);
	};

	class CollisionStay : public PrCore::Event {
	public:
		CollisionStay(const CollisionInfo& p_collisionInfo) :
			m_collisionInfo(p_collisionInfo)
		{}

		CollisionInfo m_collisionInfo;

		DEFINE_EVENT_GUID(0x9c2824c7);
	};

	class TriggerEnter : public PrCore::Event {
	public:
		TriggerEnter(const CollisionInfo& p_collisionInfo) :
			m_collisionInfo(p_collisionInfo)
		{}

		CollisionInfo m_collisionInfo;

		DEFINE_EVENT_GUID(0xba720fc1);
	};

	class TriggerExit : public PrCore::Event {
	public:
		TriggerExit(const CollisionInfo& p_collisionInfo) :
			m_collisionInfo(p_collisionInfo)
		{}

		CollisionInfo m_collisionInfo;

		DEFINE_EVENT_GUID(0xf77aa460);
	};

	// Not supported by PhysX, will implement when necessary
	//class TriggerStay : public PrCore::Event {
	//public:
	//	TriggerStay(const CollisionInfo& p_collisionInfo) :
	//		m_collisionInfo(p_collisionInfo)
	//	{}

	//	CollisionInfo m_collisionInfo;

	//	DEFINE_EVENT_GUID(0x737bc0db);
	//};

	// Constraints and OnSleep OnWake in the future
}