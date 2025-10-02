#include "Core/Common/pearl_pch.h"

#include "Physics/Resources/ConvexMeshLoader.h"
#include "Physics/Impl/PhysConvexMesh.h"
#include "Physics/Core/PhysicsSystem.h"

#include "Core/File/FileSystem.h"

#include "PhysX/PxPhysicsAPI.h"
#include "Physics/Impl/ConvertUtils.h"

using namespace PrPhysics;
using namespace physx;

PrCore::Resources::IResourceDataPtr ConvexMeshLoader::LoadResource(const std::string& p_path)
{
	auto file = PrCore::File::FileSystem::GetInstancePtr()->FileOpen(p_path);
	if (!file)
		return nullptr;

	auto size = PrCore::File::FileSystem::GetInstancePtr()->FileSize(file);
	uint8_t* buff = new uint8_t[size];
	PrCore::File::FileSystem::GetInstancePtr()->FileRead(file, buff, size);
	PrCore::File::FileSystem::GetInstancePtr()->FileClose(file);

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

	auto meshPtr = convexMeshPtr->GetMesh();
	if (meshPtr)
	{
		PxConvexMeshDesc convexDesc;
		convexDesc.points.count = static_cast<PxU32>(meshPtr->GetVerticesCount());
		convexDesc.points.stride = sizeof(PxVec3);

		const auto& verts = meshPtr->GetVertices();
		std::vector<PxVec3> vertices;
		for (auto& vert : verts)
		{
			vertices.push_back(ToPxVec3(vert));
		}

		convexDesc.points.data = vertices.data();
		convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX | PxConvexFlag::eQUANTIZE_INPUT;

		PxTolerancesScale scale;
		PxCookingParams params(scale);
		PxDefaultMemoryOutputStream outputStream;
		if (!PxCookConvexMesh(params, convexDesc, outputStream))
		{
			PRLOG_ERROR("Cannot bake the convex mesh!");
			return false;
		}

		auto file = PrCore::File::FileSystem::GetInstancePtr()->FileOpen(p_path, PrCore::File::OpenMode::Write);
		if (!file)
			return false;

		PrCore::File::FileSystem::GetInstancePtr()->FileWrite(file, outputStream.getData(), outputStream.getSize());
		PrCore::File::FileSystem::GetInstancePtr()->FileClose(file);

		return true;
	}

	return false;
}
