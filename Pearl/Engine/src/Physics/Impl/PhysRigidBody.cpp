#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/PhysShape.h"
#include "Physics/Impl/PhysRigidBody.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;
using namespace physx;

PhysRigidBodyStatic::PhysRigidBodyStatic(physx::PxRigidStatic* p_rigidStatic) :
	m_impl(p_rigidStatic)
{
	PR_ASSERT(!m_impl->userData, "PxActor::userData is not null. Something is wrong");
	m_impl->userData = new PrCore::ECS::Entity();
}

PhysRigidBodyStatic::~PhysRigidBodyStatic()
{
	delete m_impl->userData;
	m_impl->release();
}

void PhysRigidBodyStatic::ReleaseNativePtr()
{
	delete m_impl->userData;
	m_impl->release();
}

void* PhysRigidBodyStatic::GetNativePtr()
{
	return static_cast<void*>(m_impl);
}

uint32_t PhysRigidBodyStatic::GetInternalActorIndex() const
{
	return m_impl->getInternalActorIndex();
}

uint32_t PhysRigidBodyStatic::GetShapesNumber() const
{
	return m_impl->getNbShapes();
}

uint32_t PhysRigidBodyStatic::GetConstraintsNumber() const
{
	return m_impl->getNbConstraints();
}

PrPhysics::Transform PhysRigidBodyStatic::GetGlobalPose() const
{
	return ToTransform(m_impl->getGlobalPose());
}

void PhysRigidBodyStatic::SetGlobalPose(const Transform& p_pose, bool p_autowake /*= true*/)
{
	m_impl->setGlobalPose(ToPxTransform(p_pose), p_autowake);
}

bool PhysRigidBodyStatic::AttachShape(IShapePtr p_shape)
{
	return m_impl->attachShape(*static_cast<PxShape*>(p_shape->GetNativePtr()));
}

void PhysRigidBodyStatic::DetachShape(IShapePtr p_shape, bool p_wakeOnLostTouch /*= true*/)
{
	m_impl->detachShape(*static_cast<PxShape*>(p_shape->GetNativePtr()), p_wakeOnLostTouch);
}

PrPhysics::ActorType PhysRigidBodyStatic::GetType()
{
	return CastFlag<ActorType>(m_impl->getType());
}

void PhysRigidBodyStatic::SetName(const char* p_name)
{
	m_impl->setName(p_name);
}

const char* PhysRigidBodyStatic::GetName()
{
	return m_impl->getName();
}

PrPhysics::Bounds3 PhysRigidBodyStatic::GetWorldBounds(float p_inflation /*= 1.01f*/) const
{
	return ToBounds3(m_impl->getWorldBounds(p_inflation));
}

void PhysRigidBodyStatic::SetActorFlags(ActorFlag p_flag)
{
	m_impl->setActorFlags(CastFlag<PxActorFlags>(p_flag));
}

void PhysRigidBodyStatic::SetActorFlag(ActorFlag p_flag, bool p_value)
{
	m_impl->setActorFlag(CastFlag<PxActorFlag::Enum>(p_flag), p_value);
}

PrPhysics::ActorFlag PhysRigidBodyStatic::GetActorFlag()
{
	return CastFlag<ActorFlag>(m_impl->getActorFlags());
}

PrCore::ECS::Entity PhysRigidBodyStatic::GetEntity()
{
	return *static_cast<PrCore::ECS::Entity*>(m_impl->userData);
}

void PhysRigidBodyStatic::SetEntity(PrCore::ECS::Entity p_entity)
{
	*static_cast<PrCore::ECS::Entity*>(m_impl->userData) = p_entity;
}

PhysRigidBodyDynamic::PhysRigidBodyDynamic(physx::PxRigidDynamic* p_rigidDynamic) :
	m_impl(p_rigidDynamic)
{
	PR_ASSERT(!m_impl->userData, "PxActor::userData is not null. Something is wrong");
	m_impl->userData = new PrCore::ECS::Entity();
}

PhysRigidBodyDynamic::~PhysRigidBodyDynamic()
{
	delete m_impl->userData;
	m_impl->release();
}

void PhysRigidBodyDynamic::SetCMassLocalPose(const Transform& p_transform)
{
	PxTransform transform{ ToPxVec3(p_transform.position), ToPxQuat(p_transform.rotation) };
	m_impl->setCMassLocalPose(transform);
}

const PrPhysics::Transform& PhysRigidBodyDynamic::GetCMassLocalPose() const
{
	return ToTransform(m_impl->getCMassLocalPose());
}

void PhysRigidBodyDynamic::SetMass(float p_mass)
{
	m_impl->setMass(p_mass);
}

float PhysRigidBodyDynamic::GetMass() const
{
	return m_impl->getMass();
}

float PhysRigidBodyDynamic::GetInvMass() const
{
	return m_impl->getInvMass();
}

void PhysRigidBodyDynamic::SetMassSpaceInertiaTensor(const PrCore::Math::vec3& p_mass)
{
	m_impl->setMassSpaceInertiaTensor(ToPxVec3(p_mass));
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetMassSpaceInertiaTensor() const
{
	return ToVec3(m_impl->getMassSpaceInertiaTensor());
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetMassSpaceInvInertiaTensor() const
{
	return ToVec3(m_impl->getMassSpaceInvInertiaTensor());
}

float PhysRigidBodyDynamic::GetAngularLinearDamping() const
{
	return m_impl->getAngularDamping();
}

void PhysRigidBodyDynamic::SetAngularLinearDamping(float p_angularDamp)
{
	m_impl->setAngularDamping(p_angularDamp);
}

void PhysRigidBodyDynamic::SetLinearDamping(float p_linDamp)
{
	m_impl->setLinearDamping(p_linDamp);
}

float PhysRigidBodyDynamic::GetLinearDamping() const
{
	return m_impl->getLinearDamping();
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetLinearVelocity() const
{
	return ToVec3(m_impl->getLinearVelocity());
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetAngularVelocity() const
{
	return ToVec3(m_impl->getAngularVelocity());
}

void PhysRigidBodyDynamic::SetMaxLinearVelocity(float p_maxLinVel)
{
	m_impl->setMaxLinearVelocity(p_maxLinVel);
}

float PhysRigidBodyDynamic::GetMaxLinearVelocity() const
{
	return m_impl->getMaxLinearVelocity();
}

void PhysRigidBodyDynamic::SetMaxAngularVelocity(float p_maxAngularVel)
{
	m_impl->setMaxAngularVelocity(p_maxAngularVel);
}

float PhysRigidBodyDynamic::GetMaxAngularVelocity() const
{
	return m_impl->getMaxAngularVelocity();
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetLinearAcceleration() const
{
	return ToVec3(m_impl->getLinearAcceleration());
}

PrCore::Math::vec3 PhysRigidBodyDynamic::GetAngularAcceleration() const
{
	return ToVec3(m_impl->getAngularAcceleration());
}

void PhysRigidBodyDynamic::AddForce(const PrCore::Math::vec3& p_force, ForceMode p_mode /*= ForceMode::Force*/, bool p_autowake /*= true*/)
{
	m_impl->addForce(ToPxVec3(p_force), CastFlag<physx::PxForceMode::Enum>(p_mode), p_autowake);
}

void PhysRigidBodyDynamic::AddTorque(const PrCore::Math::vec3& p_torque, ForceMode p_mode /*= ForceMode::Force*/, bool p_autowake /*= true*/)
{
	m_impl->addTorque(ToPxVec3(p_torque), CastFlag<physx::PxForceMode::Enum>(p_mode), p_autowake);
}

void PhysRigidBodyDynamic::SetForceAndTorque(const PrCore::Math::vec3& p_force, const PrCore::Math::vec3& p_torque, ForceMode p_mode /*= ForceMode::Force*/)
{
	m_impl->setForceAndTorque(ToPxVec3(p_torque), ToPxVec3(p_force), CastFlag<physx::PxForceMode::Enum>(p_mode));
}

void PhysRigidBodyDynamic::ClearTorque(ForceMode p_mode /*= ForceMode::Force*/)
{
	m_impl->clearTorque(CastFlag<physx::PxForceMode::Enum>(p_mode));
}

void PhysRigidBodyDynamic::ClearForce(ForceMode p_mode /*= ForceMode::Force*/)
{
	m_impl->clearForce(CastFlag<physx::PxForceMode::Enum>(p_mode));
}

void PhysRigidBodyDynamic::SetRigidBodyFlags(RigidBodyFlag p_flag)
{
	m_impl->setRigidBodyFlags(CastFlag<PxRigidBodyFlag::Enum>(p_flag));
}

void PhysRigidBodyDynamic::SetRigidBodyFlag(RigidBodyFlag p_flag, bool p_value)
{
	m_impl->setRigidBodyFlag(CastFlag<PxRigidBodyFlag::Enum>(p_flag), p_value);
}

PrPhysics::RigidBodyFlag PhysRigidBodyDynamic::GetRigidbodyFlags() const
{
	return CastFlag<RigidBodyFlag>(m_impl->getRigidBodyFlags());
}

uint32_t PhysRigidBodyDynamic::GetShapesNumber() const
{
	return m_impl->getNbShapes();
}

uint32_t PhysRigidBodyDynamic::GetInternalActorIndex() const
{
	return m_impl->getInternalActorIndex();
}

uint32_t PhysRigidBodyDynamic::GetConstraintsNumber() const
{
	return m_impl->getNbConstraints();
}

PrPhysics::Transform PhysRigidBodyDynamic::GetGlobalPose() const
{
	return ToTransform(m_impl->getGlobalPose());
}

void PhysRigidBodyDynamic::SetGlobalPose(const Transform& p_pose, bool p_autowake /*= true*/)
{
	m_impl->setGlobalPose(ToPxTransform(p_pose), p_autowake);
}

void PhysRigidBodyDynamic::SetName(const char* p_name)
{
	m_impl->setName(p_name);
}

const char* PhysRigidBodyDynamic::GetName()
{
	return m_impl->getName();
}

bool PhysRigidBodyDynamic::AttachShape(IShapePtr p_shape)
{
	return m_impl->attachShape(*static_cast<PxShape*>(p_shape->GetNativePtr()));
}

void PhysRigidBodyDynamic::DetachShape(IShapePtr p_shape, bool p_wakeOnLostTouch /*= true*/)
{
	m_impl->detachShape(*static_cast<PxShape*>(p_shape->GetNativePtr()), p_wakeOnLostTouch);
}

PrPhysics::Bounds3 PhysRigidBodyDynamic::GetWorldBounds(float p_inflation /*= 1.01f*/) const
{
	return ToBounds3(m_impl->getWorldBounds(p_inflation));
}

PrPhysics::ActorType PhysRigidBodyDynamic::GetType()
{
	return CastFlag<ActorType>(m_impl->getType());
}

void PhysRigidBodyDynamic::SetActorFlags(ActorFlag p_flag)
{
	m_impl->setActorFlags(CastFlag<PxActorFlag::Enum>(p_flag));
}

void PhysRigidBodyDynamic::SetActorFlag(ActorFlag p_flag, bool p_value)
{
	m_impl->setActorFlag(CastFlag<PxActorFlag::Enum>(p_flag), p_value);
}

PrPhysics::ActorFlag PhysRigidBodyDynamic::GetActorFlag()
{
	return CastFlag<ActorFlag>(m_impl->getActorFlags());
}

void PhysRigidBodyDynamic::SetKinematicTarget(const Transform& p_destination)
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool PhysRigidBodyDynamic::GetKinematicTarget(Transform& p_target) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

bool PhysRigidBodyDynamic::IsSleeping() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void PhysRigidBodyDynamic::PutToSleep()
{
	throw std::logic_error("The method or operation is not implemented.");
}

void PhysRigidBodyDynamic::WakeUp()
{
	throw std::logic_error("The method or operation is not implemented.");
}

float PhysRigidBodyDynamic::GetWakeCounter() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void PhysRigidBodyDynamic::SetWakeCounter(float p_wakeCounter) const
{
	throw std::logic_error("The method or operation is not implemented.");
}

PrPhysics::RigidBodyDynamicLockFlags PhysRigidBodyDynamic::GetRigidDynamicLockFlags() const
{
	throw std::logic_error("The method or operation is not implemented.");
}

void PhysRigidBodyDynamic::SetRigidDynamicLockFlags(RigidBodyDynamicLockFlags p_lockFlags)
{
	throw std::logic_error("The method or operation is not implemented.");
}

void* PhysRigidBodyDynamic::GetNativePtr()
{
	return static_cast<void*>(m_impl);
}

void PhysRigidBodyDynamic::ReleaseNativePtr()
{
	delete m_impl->userData;
	m_impl->release();
}

PrCore::ECS::Entity PhysRigidBodyDynamic::GetEntity()
{
	return *static_cast<PrCore::ECS::Entity*>(m_impl->userData);
}

void PhysRigidBodyDynamic::SetEntity(PrCore::ECS::Entity p_entity)
{
	*static_cast<PrCore::ECS::Entity*>(m_impl->userData) = p_entity;
}
