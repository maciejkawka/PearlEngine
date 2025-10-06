#pragma once

#include "Physics/Utils/INativePtrHandler.h"
#include "Core/Resources/IResource.h"
#include "Renderer/Resources/Mesh.h"

namespace PrPhysics {

	class IConvexMesh : public PrCore::Resources::IResourceData, public PrPhysics::INativePtrHandle {
	public:
		~IConvexMesh() = default;

		virtual	size_t        GetVerticesCount() const = 0;
		virtual	size_t	      GetPolygonsCount() const = 0;
		virtual	const void*   GetVertices() const = 0;
	};
	REGISTRER_RESOURCE_HANDLE(IConvexMesh);

	using IConvexMeshPtr = std::shared_ptr<IConvexMesh>;
}