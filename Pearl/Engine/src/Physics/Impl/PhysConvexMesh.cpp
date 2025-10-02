#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/PhysConvexMesh.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;
using namespace physx;

PhysConvexMesh::PhysConvexMesh(physx::PxConvexMesh* p_convexMesh) :
	m_impl(p_convexMesh)
{
}

PhysConvexMesh::PhysConvexMesh(physx::PxConvexMesh* p_convexMesh, PrRenderer::Resources::MeshPtr p_meshPtr):
	m_impl(p_convexMesh),
	m_meshPtr(p_meshPtr)
{

}

PhysConvexMesh::~PhysConvexMesh()
{
	m_impl->release();
}

size_t PhysConvexMesh::GetVerticesCount() const
{
	return m_impl->getNbVertices();
}

size_t PhysConvexMesh::GetPolygonsCount() const
{
	return m_impl->getNbPolygons();
}

const uint8_t* PhysConvexMesh::GetVertices() const
{
	return nullptr;
}

size_t PhysConvexMesh::GetByteSize() const
{
	return 0;
}

void* PhysConvexMesh::GetNativePtr()
{
	return static_cast<void*>(m_impl);
}

void PhysConvexMesh::ReleaseNativePtr()
{
	m_impl->release();
}

PrRenderer::Resources::MeshPtr PhysConvexMesh::GetMesh() const
{
	return m_meshPtr;
}
