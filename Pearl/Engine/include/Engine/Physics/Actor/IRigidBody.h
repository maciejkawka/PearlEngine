#pragma once

#include "Physics/Actor/IActor.h"
#include "Physics/Shape/IShape.h"

namespace PrPhysics {

	enum class RigidBodyFlag {
		Kinematic                      = 1<<0,
		KinematicTargetForSceneQueries = 1<<1,
		EnableCCD                      = 1<<2,
		EnableCCDFriction              = 1<<3,
		EnableSpeculativeCCD           = 1<<4,
		EnablePoseIntegrationPreview   = 1<<5,
		EnableCCDMaxContactImpulse     = 1<<6, 
		RetainAccelerations            = 1<<7,
		ForceKinKineNotifications      = 1<<8,
		ForceStaticKineNotifications   = 1<<9,
		EnableGyroscopicForces         = 1<<10
	};
	DEFINE_ENUM_FLAG_OPERATORS(RigidBodyFlag);


	enum class RigidBodyDynamicLockFlags
	{
		LockLinearX  = (1 << 0),
		LockLinearY  = (1 << 1),
		LockLinearZ  = (1 << 2),
		LockAngularX = (1 << 3),
		LockAngularY = (1 << 4),
		LockAngularZ = (1 << 5)
	};
	DEFINE_ENUM_FLAG_OPERATORS(RigidBodyDynamicLockFlags);

	enum class ForceMode {
		Force,
		Impulsive,
		VelocityChange,
		Acceleration
	};

	class IRigidActor : public IActor {
	public:

		virtual uint32_t            GetInternalActorIndex() const = 0;
		virtual uint32_t            GetShapesNumber() const = 0;
		virtual uint32_t            GetConstraintsNumber() const = 0;

		virtual Transform           GetGlobalPose() const = 0;
		virtual void                SetGlobalPose(const Transform& p_pose, bool p_autowake = true) = 0;

		virtual bool                AttachShape(IShapePtr p_shape) = 0;
		virtual void                DetachShape(IShapePtr p_shape, bool p_wakeOnLostTouch = true) = 0;
	};
	using IRigidActorPtr = std::shared_ptr<IRigidActor>;

	class IRigidBody : public IRigidActor {
	public:
		
		virtual void                      SetCMassLocalPose(const Transform& p_transform) = 0;
		virtual const Transform&          GetCMassLocalPose() const = 0;

		virtual void                      SetMass(float p_mass) = 0;
		virtual float                     GetMass() const = 0;
		virtual float                     GetInvMass() const= 0;

		virtual void               SetMassSpaceInertiaTensor(const PrCore::Math::vec3& p_mass) = 0;
		virtual PrCore::Math::vec3 GetMassSpaceInertiaTensor() const = 0;
		virtual PrCore::Math::vec3 GetMassSpaceInvInertiaTensor() const = 0;

		virtual void                      SetLinearDamping(float p_linDamp) = 0;
		virtual float                     GetLinearDamping() const = 0;
		virtual void                      SetAngularLinearDamping(float p_linDamp) = 0;
		virtual float                     GetAngularLinearDamping() const  = 0;

		virtual PrCore::Math::vec3        GetLinearVelocity() const = 0;
		virtual PrCore::Math::vec3        GetAngularVelocity() const = 0;

		virtual void                      SetLinearVelocity(const PrCore::Math::vec3& p_velocity) = 0;
		virtual void                      SetAngularVelocity(const PrCore::Math::vec3& p_velocity) = 0;

		virtual void                      SetMaxLinearVelocity(float maxLinVel) = 0;
		virtual float                     GetMaxLinearVelocity() const = 0;
		virtual void                      SetMaxAngularVelocity(float maxLinVel) = 0;
		virtual float                     GetMaxAngularVelocity() const = 0;

		virtual PrCore::Math::vec3        GetLinearAcceleration() const = 0;
		virtual PrCore::Math::vec3        GetAngularAcceleration() const = 0;

		virtual void                      AddForce(const PrCore::Math::vec3& p_force, ForceMode p_mode = ForceMode::Force, bool p_autowake = true) = 0;
		virtual void                      AddTorque(const PrCore::Math::vec3& p_torque, ForceMode p_mode = ForceMode::Force, bool p_autowake = true) = 0;
		virtual void                      SetForceAndTorque(const PrCore::Math::vec3& p_force, const PrCore::Math::vec3& p_torque, ForceMode p_mode = ForceMode::Force) = 0;
		virtual void                      ClearForce(ForceMode p_mode = ForceMode::Force) = 0;
		virtual void                      ClearTorque(ForceMode p_mode = ForceMode::Force) = 0;

		virtual void                      SetRigidBodyFlags(RigidBodyFlag p_flag) = 0;
		virtual void                      SetRigidBodyFlag(RigidBodyFlag p_flag, bool p_value) = 0;
		virtual RigidBodyFlag             GetRigidbodyFlags() const = 0;

		virtual void                      SetMinCCDAdvanceCoefficient(float advanceCoefficient) = 0;
		virtual float                     GetMinCCDAdvanceCoefficient() const = 0;

		virtual void                      SetMaxDepenetrationVelocity(float biasClamp) = 0;
		virtual float                     GetMaxDepenetrationVelocity() const = 0;

		virtual void                      SetMaxContactImpulse(float maxImpulse) = 0;
		virtual float                     GetMaxContactImpulse() const = 0;

		virtual void                      SetContactSlopCoefficient(float slopCoefficient) = 0;
		virtual float                     GetContactSlopCoefficient() const = 0;
	};
	using IRigidBodyPtr = std::shared_ptr<IRigidBody>;

	class IRigidStatic : public IRigidActor {
	public:

	};
	using IRigidStaticPtr = std::shared_ptr<IRigidStatic>;

	class IRigidBodyDynamic : public IRigidBody {
	public:
		virtual void                      SetKinematicTarget(const Transform& p_destination) = 0;
		virtual bool                      GetKinematicTarget(Transform& p_target)	const = 0;
							              
		virtual bool                      IsSleeping() const = 0;
		virtual void                      PutToSleep() = 0;
		virtual void                      WakeUp() = 0;
							            
		virtual float                     GetWakeCounter() const = 0;
		virtual void                      SetWakeCounter(float p_wakeCounter) const = 0;

		virtual RigidBodyDynamicLockFlags GetRigidDynamicLockFlags() const = 0;
		virtual void                      SetRigidDynamicLockFlags(RigidBodyDynamicLockFlags p_lockFlags) = 0;
	};
	using IRigidBodyDynamicPtr = std::shared_ptr<IRigidBodyDynamic>;
}
