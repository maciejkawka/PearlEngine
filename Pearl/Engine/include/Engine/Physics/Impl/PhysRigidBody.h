#pragma once

#include "Physics/Actor/IRigidBody.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	static PrCore::ECS::Entity ConvertToEntity(physx::PxActor* p_physxActor)
	{
		PR_ASSERT(p_physxActor->userData, "PxActor::userData is null. Something is wrong");
		return *static_cast<PrCore::ECS::Entity*>(p_physxActor->userData);
	}

	class PhysRigidBodyStatic : public IRigidStatic {
	public:
		PhysRigidBodyStatic(physx::PxRigidStatic* p_rigidStatic);
		~PhysRigidBodyStatic();

		void* GetNativePtr() override;
		void  ReleaseNativePtr() override;

		uint32_t GetInternalActorIndex() const override;
		uint32_t GetShapesNumber() const override;
		uint32_t GetConstraintsNumber() const override;

		Transform GetGlobalPose() const override;
		void      SetGlobalPose(const Transform& p_pose, bool p_autowake = true) override;
	
		bool      AttachShape(IShapePtr p_shape) override;
		IShapePtr GetShape() override;
		void      DetachShape(IShapePtr p_shape, bool p_wakeOnLostTouch = true) override;

		void        SetName(const char* p_name) override;
		const char* GetName() override;

		Bounds3 GetWorldBounds(float p_inflation = 1.01f) const override;

		ActorType GetType() override;
		void      SetActorFlags(ActorFlag p_flag) override;
		void      SetActorFlag(ActorFlag p_flag, bool p_value) override;
		ActorFlag GetActorFlag() override;

		PrCore::ECS::Entity GetEntity() override;
		void                SetEntity(PrCore::ECS::Entity p_entity) override;

	private:
		physx::PxRigidStatic* m_impl;

		// Proxy Objects
		IShapePtr             m_shape;
	};

	class PhysRigidBodyDynamic : public IRigidBodyDynamic {
	public:
		PhysRigidBodyDynamic(physx::PxRigidDynamic* p_rigidDynamic);
		~PhysRigidBodyDynamic();

		void             SetCMassLocalPose(const Transform& p_transform) override;
		const Transform& GetCMassLocalPose() const override;

		void  SetMass(float p_mass) override;
		float GetMass() const override;
		float GetInvMass() const override;

		void               SetMassSpaceInertiaTensor(const PrCore::Math::vec3& p_mass) override;
		PrCore::Math::vec3 GetMassSpaceInertiaTensor() const override;
		PrCore::Math::vec3 GetMassSpaceInvInertiaTensor() const override;

		void  SetLinearDamping(float p_linDamp) override;
		float GetLinearDamping() const override;

		void  SetAngularLinearDamping(float p_angularDamp) override;
		float GetAngularLinearDamping() const override;

		PrCore::Math::vec3 GetLinearVelocity() const override;
		PrCore::Math::vec3 GetAngularVelocity() const override;

		void SetLinearVelocity(const PrCore::Math::vec3& p_velocity) override;
		void SetAngularVelocity(const PrCore::Math::vec3& p_velocity) override;

		void  SetMaxLinearVelocity(float p_maxLinVel) override;
		float GetMaxLinearVelocity() const override;

		void  SetMaxAngularVelocity(float p_maxAngularVel) override;
		float GetMaxAngularVelocity() const override;

		PrCore::Math::vec3 GetLinearAcceleration() const override;
		PrCore::Math::vec3 GetAngularAcceleration() const override;

		void  AddForce(const PrCore::Math::vec3& p_force, ForceMode p_mode = ForceMode::Force, bool p_autowake = true) override;
		void  AddTorque(const PrCore::Math::vec3& p_torque, ForceMode p_mode = ForceMode::Force, bool p_autowake = true) override;
		void  SetForceAndTorque(const PrCore::Math::vec3& p_force, const PrCore::Math::vec3& p_torque, ForceMode p_mode = ForceMode::Force) override;
		void  ClearForce(ForceMode p_mode = ForceMode::Force) override;
		void  ClearTorque(ForceMode p_mode = ForceMode::Force) override;

		void          SetRigidBodyFlags(RigidBodyFlag p_flag) override;
		void          SetRigidBodyFlag(RigidBodyFlag p_flag, bool p_value) override;
		RigidBodyFlag GetRigidbodyFlags() const override;

		void  SetMinCCDAdvanceCoefficient(float advanceCoefficient) override {}
		float GetMinCCDAdvanceCoefficient() const override { return 0; }

		void  SetMaxDepenetrationVelocity(float biasClamp) override {}
		float GetMaxDepenetrationVelocity() const override { return 0; }

		void   SetMaxContactImpulse(float maxImpulse) override {}
		float  GetMaxContactImpulse() const override { return 0; }

		void  SetContactSlopCoefficient(float slopCoefficient) override {}
		float GetContactSlopCoefficient() const override { return 0; }

		uint32_t GetInternalActorIndex() const override;
		uint32_t GetShapesNumber() const override;
		uint32_t GetConstraintsNumber() const override;

		Transform GetGlobalPose() const override;
		void      SetGlobalPose(const Transform& p_pose, bool p_autowake = true) override;

		bool      AttachShape(IShapePtr p_shape) override;
		IShapePtr GetShape() override;
		void      DetachShape(IShapePtr p_shape, bool p_wakeOnLostTouch = true) override;

		void        SetName(const char* p_name) override;
		const char* GetName() override;

		Bounds3   GetWorldBounds(float p_inflation = 1.01f) const override;

		ActorType GetType() override;
		void      SetActorFlags(ActorFlag p_flag) override;
		void      SetActorFlag(ActorFlag p_flag, bool p_value) override;
		ActorFlag GetActorFlag() override;

		void SetKinematicTarget(const Transform& p_destination) override;
		bool GetKinematicTarget(Transform& p_target) const override;

		bool  IsSleeping() const override;
		void  PutToSleep() override;
		void  WakeUp() override;
		float GetWakeCounter() const override;
		void  SetWakeCounter(float p_wakeCounter) const override;

		RigidBodyDynamicLockFlags GetRigidDynamicLockFlags() const override;
		void                      SetRigidDynamicLockFlags(RigidBodyDynamicLockFlags p_lockFlags) override;

		void* GetNativePtr() override;
		void  ReleaseNativePtr() override;

		PrCore::ECS::Entity GetEntity() override;
		void                SetEntity(PrCore::ECS::Entity p_entity) override;

	private:
		physx::PxRigidDynamic* m_impl;

		// Proxy Objects
		IShapePtr m_shape;
	};


}