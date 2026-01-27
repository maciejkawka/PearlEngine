#include "Core/Common/pearl_pch.h"

#include "Physics/Resources/TriangleMeshLoader.h"

#include "Physics/Core/PhysicsSystem.h"
#include "Physics/Impl/PhysTriangleMesh.h"
#include "Physics/Impl/ConvertUtils.h"

#include "Core/File/FileSystem.h"

#include "PhysX/PxPhysicsAPI.h"

using namespace PrPhysics;
using namespace physx;

PrCore::IResourceDataPtr TriangleMeshLoader::LoadResource(const std::string& p_path)
{
	auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
	auto file = pFileSystem->FileOpen(p_path);
	if (!file)
		return nullptr;

	auto size = pFileSystem->FileSize(file);
	uint8_t* buff = new uint8_t[size];
	pFileSystem->FileRead(file, buff, size);
	pFileSystem->FileClose(file);

	auto convexMeshPtr = PrSystems::Get<PhysicsSystem>()->CreateTriangleMesh(buff, size);
	delete[] buff;

	return convexMeshPtr;
}

void TriangleMeshLoader::UnloadResource(PrCore::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool TriangleMeshLoader::SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	ITriangleMeshPtr triangleMeshPtr = std::static_pointer_cast<ITriangleMesh>(p_resourceData);

	PxTriangleMeshDesc triangleDesc;
	triangleDesc.points.count = static_cast<PxU32>(triangleMeshPtr->GetVerticesCount());
	triangleDesc.points.stride = sizeof(PxVec3);
	triangleDesc.points.data = triangleMeshPtr->GetVertices();

	triangleDesc.triangles.count = static_cast<PxU32>(triangleMeshPtr->GetTrianglesCount());
	triangleDesc.triangles.stride = 3 * sizeof(PxU32);

	// Must convert triangles if 16 bit
	std::vector<PxU32> tris32;
	if ((triangleMeshPtr->GetFlags() & TriangleMeshFlags::Indices16Bit) == TriangleMeshFlags::Indices16Bit)
	{
		tris32.resize(triangleMeshPtr->GetTrianglesCount() * 3);
		const PxU16* tris16 = static_cast<const PxU16*>(triangleMeshPtr->GetTriangles());
		for (PxU32 i = 0; i < triangleMeshPtr->GetTrianglesCount() * 3; ++i)
		{
			tris32[i] = static_cast<PxU32>(tris16[i]);
		}

		triangleDesc.triangles.data = tris32.data();
	}
	else
	{
		triangleDesc.triangles.data = triangleMeshPtr->GetTriangles();
	}

	PxTolerancesScale scale;
	PxCookingParams params(scale);
	PxDefaultMemoryOutputStream outputStream;
	if (!PxCookTriangleMesh(params, triangleDesc, outputStream))
	{
		PRLOG_ERROR("Cannot bake the convex mesh!");
		return false;
	}

	auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
	auto file = pFileSystem->FileOpen(p_path, PrCore::FileOpenMode::Write);
	if (!file)
		return false;

	pFileSystem->FileWrite(file, outputStream.getData(), outputStream.getSize());
	pFileSystem->FileClose(file);

	return true;
}