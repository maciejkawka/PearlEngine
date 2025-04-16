#pragma once

#include "Physics/Utils/PhysicsMath.h"
#include "Physics/Utils/INativePtrHandler.h"

#include "Core/ECS/EntityManager.h"

namespace PrPhysics {

	enum class ActorType {
		RigidStatic = 0,
		RigidDynamic = 1,
		ParticleSystem = 5
	};

	enum class ActorFlag {
		Visalization = 1 << 0,
		DisableGravity = 1 << 1,
		SleepNotifies = 1 << 2,
		Simulation = 1 << 3
	};
	DEFINE_ENUM_FLAG_OPERATORS(ActorFlag);

	class IActor: public INativePtrHandle {
	public:
		virtual ActorType    GetType() = 0;

		virtual	void         SetName(const char* p_name) = 0;
		virtual const char*  GetName() = 0;

		virtual Bounds3      GetWorldBounds(float p_inflation = 1.01f) const = 0;

		virtual	void         SetActorFlags(ActorFlag p_flag) = 0;
		virtual void         SetActorFlag(ActorFlag p_flag, bool p_value) = 0;
		virtual ActorFlag    GetActorFlag() = 0;

		virtual PrCore::ECS::Entity GetEntity() = 0;
		virtual void                SetEntity(PrCore::ECS::Entity p_entity) = 0;
	};
	using IActorPtr = std::shared_ptr<IActor>;
}