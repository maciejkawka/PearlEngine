#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/ConvertUtils.h"
#include "Physics/Impl/PhysFactory.h"
#include "Physics/Impl/PhysRigidBody.h"
#include "Physics/Impl/PhysShape.h"

using namespace PrPhysics;
using namespace physx;

PhysFactory::PhysFactory(physx::PxPhysics* p_physics):
	m_physics(p_physics)
{
	PR_ASSERT(m_physics, "Phisics is nullptr!");
}

PrPhysics::IRigidStaticPtr PhysFactory::CreateRigidStatic(const Transform& p_transform)
{
	auto pxRigidStatic = m_physics->createRigidStatic(ToPxTransform(p_transform));
	return  std::make_shared<PhysRigidBodyStatic>(pxRigidStatic);
}

PrPhysics::IRigidBodyDynamicPtr PhysFactory::CreateRigidDynamic(const Transform& p_transform)
{
	auto pxRigidBodyDynamic = m_physics->createRigidDynamic(ToPxTransform(p_transform));
	return std::make_shared<PhysRigidBodyDynamic>(pxRigidBodyDynamic);
}

PrPhysics::IShapePtr PhysFactory::CreateShape(const IGeometry& p_geometery, const Material& p_mat, bool p_isExclusive /*= false*/, ShapeFlags p_flags /*= ShapeFlags::Visualization | ShapeFlags::SceneQuery | ShapeFlags::Simulation*/)
{
	PxMaterial* material = m_physics->createMaterial(p_mat.staticFriction, p_mat.dynamicFriction, p_mat.restitution);
	material->setDamping(p_mat.dumping);

	PxShape* pxShape = m_physics->createShape(ToPxGeometry(&p_geometery).any(), *material, p_isExclusive, CastFlag<PxShapeFlag::Enum>(p_flags));
	return std::make_shared<PhysShape>(pxShape);
}
