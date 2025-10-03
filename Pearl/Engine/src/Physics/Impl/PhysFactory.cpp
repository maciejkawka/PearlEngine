#include "Core/Common/pearl_pch.h"

#include "Physics/Impl/ConvertUtils.h"
#include "Physics/Impl/PhysFactory.h"
#include "Physics/Impl/PhysRigidBody.h"
#include "Physics/Impl/PhysShape.h"
#include "Physics/Impl/PhysConvexMesh.h"

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

	PxShape* pxShape = m_physics->createShape(ToPxGeometry(p_geometery).any(), *material, p_isExclusive, CastFlag<PxShapeFlag::Enum>(p_flags));
	return std::make_shared<PhysShape>(pxShape, p_geometery, p_mat);
}

PrPhysics::IConvexMeshPtr PhysFactory::CreateConvexMesh(uint8_t* p_data, size_t p_size)
{
	PxDefaultMemoryInputData input(p_data, p_size);
	PxConvexMesh* convexMesh = m_physics->createConvexMesh(input);

	return std::make_shared<PhysConvexMesh>(convexMesh);
}

PrPhysics::IConvexMeshPtr PhysFactory::CreateConvexMesh(PrRenderer::Resources::MeshHandle p_mesh)
{
	auto verts = p_mesh->GetVertices();
	std::vector<PxVec3> vertices;
	for (auto& vert : verts)
	{
		vertices.push_back(ToPxVec3(vert));
	}

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = static_cast<PxU32>(p_mesh->GetVerticesCount());
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = vertices.data();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eQUANTIZE_INPUT;

	PxTolerancesScale scale;
	PxCookingParams params(scale);
	PxDefaultMemoryOutputStream outputStream;
	if (!PxCookConvexMesh(params, convexDesc, outputStream))
	{
		PRLOG_ERROR("Cannot bake the mesh to IConvexMesh!");
		return nullptr;
	}

	PxDefaultMemoryInputData input(outputStream.getData(), outputStream.getSize());
	PxConvexMesh* convexMesh = m_physics->createConvexMesh(input);

	return std::make_shared<PhysConvexMesh>(convexMesh, p_mesh.GetData());
}
