#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/PhysTriangleMesh.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;

PhysTriangleMesh::PhysTriangleMesh(physx::PxTriangleMesh* p_triangleMesh):
	m_impl(p_triangleMesh)
{
}

PhysTriangleMesh::~PhysTriangleMesh()
{
}

size_t PhysTriangleMesh::GetVerticesCount() const
{
	return m_impl->getNbVertices();
}

size_t PhysTriangleMesh::GetTrianglesCount() const
{
	return m_impl->getNbTriangles();
}

const const void* PhysTriangleMesh::GetVertices() const
{
	return m_impl->getVertices();
}

const const void* PhysTriangleMesh::GetTriangles() const
{
	return m_impl->getTriangles();
}

PrPhysics::TriangleMeshFlags PhysTriangleMesh::GetFlags() const
{
	return CastFlag<TriangleMeshFlags>(m_impl->getTriangleMeshFlags());
}

size_t PhysTriangleMesh::GetByteSize() const
{
	return 0;
}

void* PhysTriangleMesh::GetNativePtr()
{
	return static_cast<void*>(m_impl);
}

void PhysTriangleMesh::ReleaseNativePtr()
{
	m_impl->release();
}
