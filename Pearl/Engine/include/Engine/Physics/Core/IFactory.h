#pragma once

#include "Physics/Core/Material.h"
#include "Physics/Actor/IRigidBody.h"
#include "Physics/Shape/IGeometry.h"
#include "Physics/Shape/IShape.h"
#include "Physics/Utils/PhysicsMath.h"

namespace PrPhysics {

	class IRigidStatic;
	class IRigidBodyDynamic;

	class IFactory {
	public:
		virtual IRigidStaticPtr      CreateRigidStatic(const Transform& p_transform) = 0;
		virtual IRigidBodyDynamicPtr CreateRigidDynamic(const Transform& p_transform) = 0;

		virtual IShapePtr            CreateShape(const IGeometry& p_geometery, const Material& p_mat, bool isExclusive = false, ShapeFlags p_flags = ShapeFlags::Visualization | ShapeFlags::SceneQuery | ShapeFlags::Simulation) = 0;
	};
}