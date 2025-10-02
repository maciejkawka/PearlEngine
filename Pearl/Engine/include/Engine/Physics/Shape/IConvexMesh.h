#pragma once

#include "Physics/Utils/INativePtrHandler.h"
#include "Core/Resources/IResource.h"
#include "Renderer/Resources/Mesh.h"

namespace PrPhysics {

	class IConvexMesh : public PrCore::Resources::IResourceData, public PrPhysics::INativePtrHandle {
	public:
		virtual	size_t                         GetVerticesCount() const = 0;
		virtual	size_t	                       GetPolygonsCount() const = 0;
		virtual	const uint8_t*                 GetVertices() const = 0;
		virtual PrRenderer::Resources::MeshPtr GetMesh() const = 0;
	};
	REGISTRER_RESOURCE_HANDLE(IConvexMesh);

	using IConvexMeshPtr = std::shared_ptr<IConvexMesh>;

}