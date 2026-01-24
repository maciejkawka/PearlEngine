#pragma once

#include "Physics/Core/IFactory.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	class PhysFactory : public IFactory {
	public:
		PhysFactory(physx::PxPhysics* p_physics);

		IRigidStaticPtr      CreateRigidStatic(const Transform& p_transform) override;
		IRigidBodyDynamicPtr CreateRigidDynamic(const Transform& p_transform) override;
		IShapePtr            CreateShape(const IGeometry& p_geometery, const Material& p_mat, bool isExclusive = false, ShapeFlags p_flags = ShapeFlags::Visualization | ShapeFlags::SceneQuery | ShapeFlags::Simulation) override;
		IConvexMeshPtr       CreateConvexMesh(PrRenderer::MeshPtr p_mesh) override;
		IConvexMeshPtr       CreateConvexMesh(uint8_t* p_data, size_t p_size) override;
		ITriangleMeshPtr     CreateTriangleMesh(PrRenderer::MeshPtr p_mesh) override;
		ITriangleMeshPtr     CreateTriangleMesh(uint8_t* p_data, size_t p_size) override;

	private:
		physx::PxPhysics* m_physics;
	};
}