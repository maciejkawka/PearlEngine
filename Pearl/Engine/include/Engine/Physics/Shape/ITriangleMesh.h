#pragma once

#include "Physics/Utils/INativePtrHandler.h"
#include "Core/Resources/IResource.h"

namespace PrPhysics {

	enum class TriangleMeshFlags
	{
		Indices16Bit = (1 << 1),
		AdjacencyInfo = (1 << 2),
		PreferNoSdfProj = (1 << 3)
	};
	DEFINE_ENUM_FLAG_OPERATORS(TriangleMeshFlags);

	class ITriangleMesh : public PrCore::IResourceData, public PrPhysics::INativePtrHandle {
	public:
		virtual ~ITriangleMesh() = default;

		virtual	size_t                         GetVerticesCount() const = 0;
		virtual	size_t	                       GetTrianglesCount() const = 0;
		virtual	const void*                    GetVertices() const = 0;
		virtual	const void*                    GetTriangles() const = 0;
		virtual	TriangleMeshFlags              GetFlags() const = 0;
	};
	REGISTRER_RESOURCE_HANDLE(ITriangleMesh);

	using ITriangleMeshPtr = std::shared_ptr<ITriangleMesh>;
}