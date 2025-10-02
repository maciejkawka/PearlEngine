#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/PhysShape.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;
using namespace physx;


PhysShape::PhysShape(physx::PxShape* p_shape, const IGeometry& p_geometry) :
	m_impl(p_shape)
{
	m_visitor = GeometeryVisitor
	{
		[&](const SphereGeometry* p_geometery) {m_geometery = std::make_unique<SphereGeometry>(p_geometery->radius); },
		[&](const PlaneGeometry* p_geometery) {m_geometery = std::make_unique<PlaneGeometry>(); },
		[&](const CapsuleGeometry* p_geometery) {m_geometery = std::make_unique<CapsuleGeometry>(p_geometery->radius, p_geometery->halfHeight); },
		[&](const BoxGeometery* p_geometery) {m_geometery = std::make_unique<BoxGeometery>(p_geometery->halfExtents); },
		[&](const ConvexGeometry* p_geometery) {m_geometery = std::make_unique<ConvexGeometry>(p_geometery->convexMeshPtr, p_geometery->scale); }
	};
	
	p_geometry.Accept(m_visitor);
}

PhysShape::~PhysShape()
{
	m_impl->release();
}

void PhysShape::SetGeometry(const IGeometry& p_geometry)
{
	p_geometry.Accept(m_visitor);
	m_impl->setGeometry(ToPxGeometry(p_geometry).any());
}

const PrPhysics::IGeometry* PhysShape::GetGeometry() const
{
	return m_geometery.get();
}

void PhysShape::SetLocalPose(const Transform& p_transform)
{
	m_impl->setLocalPose(ToPxTransform(p_transform));
}

const PrPhysics::Transform& PhysShape::GetLocalPose() const
{
	return ToTransform(m_impl->getLocalPose());
}

void PhysShape::SetContactOffset(float p_contactOffset)
{
	m_impl->setContactOffset(p_contactOffset);
}

float PhysShape::GetContactOffset() const
{
	return m_impl->getContactOffset();
}

void PhysShape::SetFlags(ShapeFlags p_flag)
{
	m_impl->setFlags(CastFlag<PxShapeFlag::Enum>(p_flag));
}

PrPhysics::ShapeFlags PhysShape::GetFlags() const
{
	return CastFlag<ShapeFlags>(m_impl->getFlags());
}

void PhysShape::SetName(const char* p_name)
{
	m_impl->setName(p_name);
}

const char* PhysShape::GetName() const
{
	return m_impl->getName();
}

void* PhysShape::GetNativePtr()
{
	return static_cast<void*>(m_impl);
}

void PhysShape::ReleaseNativePtr()
{
	m_impl->release();
}

void PhysShape::SetFlag(ShapeFlags p_flag, bool p_value)
{
	m_impl->setFlag(CastFlag<PxShapeFlag::Enum>(p_flag), p_value);
}

void PhysShape::AssignGeometery(const IGeometry* p_geometry)
{
	GeometeryVisitor visitor
	{
		[&](const SphereGeometry* p_geometery) {m_geometery = std::make_unique<SphereGeometry>(p_geometery->radius); },
		[&](const PlaneGeometry* p_geometery) {m_geometery = std::make_unique<PlaneGeometry>(); },
		[&](const CapsuleGeometry* p_geometery) {m_geometery = std::make_unique<CapsuleGeometry>(p_geometery->radius, p_geometery->halfHeight); },
		[&](const BoxGeometery* p_geometery) {m_geometery = std::make_unique<BoxGeometery>(p_geometery->halfExtents); },
		[&](const ConvexGeometry* p_geometery) {m_geometery = std::make_unique<ConvexGeometry>(p_geometery->convexMeshPtr, p_geometery->scale); }
	};

	p_geometry->Accept(visitor);
}
