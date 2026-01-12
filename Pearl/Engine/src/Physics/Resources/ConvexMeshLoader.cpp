#include "Core/Common/pearl_pch.h"

#include "Physics/Core/PhysicsSystem.h"
#include "Physics/Resources/ConvexMeshLoader.h"
#include "Physics/Impl/PhysConvexMesh.h"
#include "Physics/Impl/ConvertUtils.h"

#include "Core/File/FileSystem.h"

#include "PhysX/PxPhysicsAPI.h"

using namespace PrPhysics;
using namespace physx;

PrCore::Resources::IResourceDataPtr ConvexMeshLoader::LoadResource(const std::string& p_path)
{
	auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
	auto file = pFileSystem->FileOpen(p_path);
	if (!file)
		return nullptr;

	auto size = pFileSystem->FileSize(file);
	uint8_t* buff = new uint8_t[size];
	pFileSystem->FileRead(file, buff, size);
	pFileSystem->FileClose(file);

	auto convexMeshPtr = PhysicsSystem::GetInstancePtr()->CreateConvexMesh(buff, size);
	delete[] buff;

	return convexMeshPtr;
}

void ConvexMeshLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool ConvexMeshLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	IConvexMeshPtr convexMeshPtr = std::static_pointer_cast<IConvexMesh>(p_resourceData);

	PxConvexMeshDesc convexDesc;
	convexDesc.points.count = static_cast<PxU32>(convexMeshPtr->GetVerticesCount());
	convexDesc.points.stride = sizeof(PxVec3);
	convexDesc.points.data = convexMeshPtr->GetVertices();
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eQUANTIZE_INPUT;

	PxTolerancesScale scale;
	PxCookingParams params(scale);
	PxDefaultMemoryOutputStream outputStream;
	if (!PxCookConvexMesh(params, convexDesc, outputStream))
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
