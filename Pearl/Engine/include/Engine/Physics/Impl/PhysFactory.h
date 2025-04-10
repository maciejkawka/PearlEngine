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

	private:
		physx::PxPhysics* m_physics;
	};

}