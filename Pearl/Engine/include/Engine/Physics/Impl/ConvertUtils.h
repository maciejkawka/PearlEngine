#pragma once

#include "Core/Math/Math.h"
#include "Core/Utils/Assert.h"

#include "Physics/Shape/IGeometry.h"
#include "Physics/Utils/PhysicsMath.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	inline physx::PxVec2 ToPxVec2(const PrCore::Math::vec2& p_vec2)
	{
		return physx::PxVec2(p_vec2.x, p_vec2.y);
	}

	inline physx::PxVec3 ToPxVec3(const PrCore::Math::vec3& p_vec3)
	{
		return physx::PxVec3(p_vec3.x, p_vec3.y, p_vec3.z);
	}

	inline physx::PxVec4 ToPxVec4(const PrCore::Math::vec4& p_vec4)
	{
		return physx::PxVec4(p_vec4.x, p_vec4.y, p_vec4.z, p_vec4.w);
	}

	inline physx::PxQuat ToPxQuat(const PrCore::Math::quat& p_quat)
	{
		return physx::PxQuat(p_quat.x, p_quat.y, p_quat.z, p_quat.w);
	}

	inline physx::PxTransform ToPxTransform(const PrPhysics::Transform& p_transform)
	{
		return physx::PxTransform{ ToPxVec3(p_transform.position), ToPxQuat(p_transform.rotation) };
	}

	inline  physx::PxBounds3 ToPxBounds3(const PrPhysics::Bounds3 p_bounds)
	{
		return  physx::PxBounds3{ ToPxVec3(p_bounds.min), ToPxVec3(p_bounds.max) };
	}

	inline PrCore::Math::vec2 ToVec2(const physx::PxVec2& p_pxVec)
	{
		return PrCore::Math::vec2(p_pxVec.x, p_pxVec.y);
	}

	inline PrCore::Math::vec3 ToVec3(const physx::PxVec3& p_pxVec)
	{
		return PrCore::Math::vec3(p_pxVec.x, p_pxVec.y, p_pxVec.z);
	}

	inline PrCore::Math::vec4 ToVec4(const physx::PxVec4& p_pxVec)
	{
		return PrCore::Math::vec4(p_pxVec.x, p_pxVec.y, p_pxVec.z, p_pxVec.w);
	}

	inline PrCore::Math::quat ToQuat(const physx::PxQuat& p_pxQuat)
	{
		return PrCore::Math::quat(p_pxQuat.w, p_pxQuat.x, p_pxQuat.y, p_pxQuat.z);
	}

	inline PrPhysics::Transform ToTransform(const physx::PxVec3& p_pos, const physx::PxQuat& p_rotation)
	{
		return PrPhysics::Transform{ ToQuat(p_rotation), ToVec3(p_pos) };
	}

	inline PrPhysics::Transform ToTransform(const physx::PxTransform& p_transform)
	{
		return PrPhysics::Transform{ ToQuat(p_transform.q), ToVec3(p_transform.p) };
	}

	inline PrPhysics::Bounds3 ToBounds3(const physx::PxBounds3 p_bounds)
	{
		return PrPhysics::Bounds3{ ToVec3(p_bounds.minimum), ToVec3(p_bounds.maximum) };
	}

	inline physx::PxGeometryHolder ToPxGeometry(const IGeometry& p_geometry)
	{
		physx::PxGeometryHolder holder;
		GeometeryVisitor visitor{
			[&](const SphereGeometry* p_geometery)  { holder.storeAny(physx::PxSphereGeometry{ p_geometery->radius }); },
			[&](const PlaneGeometry* p_geometery)   { holder.storeAny(physx::PxPlaneGeometry{}); },
			[&](const CapsuleGeometry* p_geometery) { holder.storeAny(physx::PxCapsuleGeometry{ p_geometery->radius, p_geometery->halfHeight }); },
			[&](const BoxGeometery* p_geometery)    { holder.storeAny(physx::PxBoxGeometry{ ToPxVec3(p_geometery->halfExtents) }); },
			[&](const ConvexGeometry* p_geometery)
			{
				physx::PxMeshScale scale;
				scale.scale = ToPxVec3(p_geometery->scale);
				holder.storeAny(physx::PxConvexMeshGeometry{ static_cast<physx::PxConvexMesh*>(p_geometery->convexMeshHandle.GetData()->GetNativePtr()), scale });
			}
		};

		p_geometry.Accept(visitor);
		return holder;
	}

	// We assume that Pearl Physics Flag have the same values as PhysX flags, might not work if something messed up.
	template<typename output, typename input>
	inline output CastFlag(input p_inputFlag)
	{
		return static_cast<output>((uint32_t)(p_inputFlag));
	}
}