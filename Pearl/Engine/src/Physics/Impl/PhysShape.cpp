#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/PhysShape.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;
using namespace physx;


PhysShape::PhysShape(physx::PxShape* p_shape) :
	m_impl(p_shape)
{
}

void PhysShape::SetGeometry(const IGeometry& p_geometry)
{
	m_impl->setGeometry(ToPxGeometry(&p_geometry).any());
}

const PrPhysics::IGeometry& PhysShape::GetGeometry() const
{
	return  PrPhysics::IGeometry{};
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

void* PhysShape::ReleaseNativePtr()
{
	m_impl->release();
	m_impl = nullptr;

	return nullptr;
}
