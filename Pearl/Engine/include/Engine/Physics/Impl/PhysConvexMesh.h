#pragma once

#include "Physics/Shape/IConvexMesh.h"

#include "PhysX/PxPhysicsAPI.h"

namespace PrPhysics {

	class PhysConvexMesh : public PrPhysics::IConvexMesh {
	public:
		PhysConvexMesh(physx::PxConvexMesh* p_convexMesh);
		PhysConvexMesh(physx::PxConvexMesh* p_convexMesh, PrRenderer::Resources::MeshPtr p_meshPtr);
		~PhysConvexMesh();

		virtual	size_t         GetVerticesCount() const override;
		virtual	size_t	       GetPolygonsCount() const override;
		virtual const uint8_t* GetVertices() const override;

		PrRenderer::Resources::MeshPtr GetMesh() const;

		size_t GetByteSize() const override;

		void* GetNativePtr() override;
		void  ReleaseNativePtr() override;

	private:
		physx::PxConvexMesh* m_impl;
		PrRenderer::Resources::MeshPtr m_meshPtr;
	};
}