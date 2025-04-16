#pragma once

#include "Physics/Impl/PhysRigidBody.h"
#include "Physics/Events/Events.h"

#include "PhysX/PxPhysicsAPI.h"

#include "Core/Events/EventManager.h"
#include "Core/ECS/Components/CoreComponents.h"

namespace PrPhysics {

	using namespace physx;

	PxFilterFlags ContactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
		PxFilterObjectAttributes attributes1, PxFilterData filterData1,
		PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
	{
		pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT | PxPairFlag::eTRIGGER_DEFAULT
			| PxPairFlag::eNOTIFY_TOUCH_FOUND
			| PxPairFlag::eNOTIFY_TOUCH_PERSISTS
			| PxPairFlag::eNOTIFY_TOUCH_LOST
			| PxPairFlag::eNOTIFY_CONTACT_POINTS;

		return PxFilterFlag::eDEFAULT;
	}

	class CollisionDispatcher: public physx::PxSimulationEventCallback {
	public:
		void onContact(const physx::PxContactPairHeader& pairHeader, const physx::PxContactPair* pairs, physx::PxU32 nbPairs) override
		{
			for (PxU32 i = 0; i < nbPairs; ++i)
			{
				const physx::PxContactPair& pair = pairs[i];
				physx::PxActor* actorA = pairHeader.actors[0];
				physx::PxActor* actorB = pairHeader.actors[1];

				CollisionInfo collisionInfo;
				collisionInfo.entityA = *static_cast<PrCore::ECS::Entity*>(actorA->userData);
				collisionInfo.entityB = *static_cast<PrCore::ECS::Entity*>(actorB->userData);

				if (pair.events & PxPairFlag::Enum::eNOTIFY_TOUCH_FOUND)
				{
					PrCore::Events::EventPtr eventPtr = std::make_shared<PrPhysics::CollisionEnter>(collisionInfo);
					PrCore::Events::EventManager::GetInstancePtr()->FireEvent(eventPtr);
				}
				else if (pair.events & PxPairFlag::Enum::eNOTIFY_TOUCH_PERSISTS)
				{
					PrCore::Events::EventPtr eventPtr = std::make_shared<PrPhysics::CollisionStay>(collisionInfo);
					PrCore::Events::EventManager::GetInstancePtr()->FireEvent(eventPtr);
				}
				else if (pair.events & PxPairFlag::Enum::eNOTIFY_TOUCH_LOST)
				{
					PrCore::Events::EventPtr eventPtr = std::make_shared<PrPhysics::CollisionExit>(collisionInfo);
					PrCore::Events::EventManager::GetInstancePtr()->FireEvent(eventPtr);
				}
			}
		}

		void onTrigger(physx::PxTriggerPair* pairs, physx::PxU32 count) override
		{
			for (PxU32 i = 0; i < count; ++i)
			{
				const physx::PxTriggerPair& pair = pairs[i];
				physx::PxActor* actorA = pair.triggerActor;
				physx::PxActor* actorB = pair.otherActor;

				CollisionInfo collisionInfo;
				collisionInfo.entityA = *static_cast<PrCore::ECS::Entity*>(actorA->userData);
				collisionInfo.entityB = *static_cast<PrCore::ECS::Entity*>(actorB->userData);

				if (pair.status & PxPairFlag::Enum::eNOTIFY_TOUCH_FOUND)
				{
					PrCore::Events::EventPtr eventPtr = std::make_shared<PrPhysics::TriggerEnter>(collisionInfo);
					PrCore::Events::EventManager::GetInstancePtr()->FireEvent(eventPtr);
				}
				else if (pair.status & PxPairFlag::Enum::eNOTIFY_TOUCH_LOST)
				{
					PrCore::Events::EventPtr eventPtr = std::make_shared<PrPhysics::TriggerExit>(collisionInfo);
					PrCore::Events::EventManager::GetInstancePtr()->FireEvent(eventPtr);
				}
			}
		}

		//Unused yet
		void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) override {}
		void onWake(physx::PxActor** actors, physx::PxU32 count) override {}
		void onSleep(physx::PxActor** actors, physx::PxU32 count) override {}
		void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) override {}
	};
}