#pragma once

#include "Physics/Shape/ITriangleMesh.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	class PhysTriangleMesh : public PrPhysics::ITriangleMesh {
	public:
		PhysTriangleMesh(physx::PxTriangleMesh* p_triangleMesh);
		~PhysTriangleMesh();

		size_t               GetVerticesCount() const override;
		size_t	             GetTrianglesCount() const override;
		const void*          GetVertices() const override;
		const void*          GetTriangles() const override;
		TriangleMeshFlags    GetFlags() const override;

		size_t GetByteSize() const override;

		void* GetNativePtr() override;
		void  ReleaseNativePtr() override;

	private:
		physx::PxTriangleMesh*         m_impl;
	};
}