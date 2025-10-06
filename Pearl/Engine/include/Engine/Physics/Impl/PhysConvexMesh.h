#pragma once

#include "Physics/Shape/IConvexMesh.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	class PhysConvexMesh : public PrPhysics::IConvexMesh {
	public:
		PhysConvexMesh(physx::PxConvexMesh* p_convexMesh);
		~PhysConvexMesh();

		virtual	size_t       GetVerticesCount() const override;
		virtual	size_t	     GetPolygonsCount() const override;
		virtual const void*  GetVertices() const override;

		size_t GetByteSize() const override;

		void* GetNativePtr() override;
		void  ReleaseNativePtr() override;

	private:
		physx::PxConvexMesh* m_impl;
	};
}