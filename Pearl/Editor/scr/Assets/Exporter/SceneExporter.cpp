#include "Editor/Assets/Exporter/SceneExporter.h"

#include "Core/Resources/ResourceSystem.h"
#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"
#include "Core/Utils/SystemProvider.h"

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Texture2DLoader.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/MaterialLoader.h"

#include "Physics/Resources/ConvexMeshLoader.h"
#include "Physics/Shape/IShape.h"

using namespace PrEditor::Assets;
using namespace PrCore;

void BasicMeshExport(ResourceDescConstPtr p_resDesc, std::string_view p_exportRoot)
{
	if (p_resDesc->origin == ResourceOrigin::Memory)
	{
		auto lambda = [=]()
		{
			auto& resourceName = p_resDesc->origin == ResourceOrigin::File ? PrCore::PathUtils::GetFile(p_resDesc->filePath) : p_resDesc->data->GetName();
			std::string path;
			if (!p_exportRoot.empty())
			{
				path = PrCore::PathUtils::MakePath(p_exportRoot, resourceName);
			}
			path += ".obj";

			if (PrSystems::Get<PrCore::FileSystem>()->FileExist(path))
				return;

			PrRenderer::MeshOBJLoader loader;
			auto success = loader.SaveResourceOnDisc(p_resDesc->data, path);
			if (!success)
			{
				PR_ASSERT(false, "Cannot export resource!");
				return;
			}

			p_resDesc->origin = ResourceOrigin::File;
			p_resDesc->state = ResourceState::Loaded;
			p_resDesc->filePath = path;
		};

		PrSystems::Get<PrCore::JobSystem>()->Schedule("Export Mesh", lambda);
	}
}

void BasicTextureExport(ResourceDescConstPtr p_resDesc, std::string_view p_exportRoot)
{
	if (p_resDesc->origin == ResourceOrigin::Memory)
	{
		auto texturePtr = std::static_pointer_cast<PrRenderer::Texture>(p_resDesc->data);
		auto data = texturePtr->FetchGPUData();
		texturePtr->SetData(data);
		texturePtr->SetReadable(true);

		auto lambda = [=]()
		{
			auto& resourceName = p_resDesc->origin == ResourceOrigin::File ? PrCore::PathUtils::GetFile(p_resDesc->filePath) : p_resDesc->data->GetName();
			std::string path;
			if (!p_exportRoot.empty())
			{
				path = PrCore::PathUtils::MakePath(p_exportRoot, resourceName);
			}
			path += ".tga";

			if (PrSystems::Get<PrCore::FileSystem>()->FileExist(path))
				return;

			PrRenderer::Texture2DLoader loader;
			auto success = loader.SaveResourceOnDisc(p_resDesc->data, path);
			if (!success)
			{
				PR_ASSERT(false, "Cannot export resource!");
				return;
			}

			p_resDesc->origin = ResourceOrigin::File;
			p_resDesc->state = ResourceState::Loaded;
			p_resDesc->filePath = path;
		};

		PrSystems::Get<PrCore::JobSystem>()->Schedule("Export Texture", lambda);
	}
}

void BasicMaterialExport(ResourceDescConstPtr p_resDesc, std::string_view p_exportRoot)
{
	if (p_resDesc->origin == ResourceOrigin::Memory)
	{
		auto resourceName = p_resDesc->origin == ResourceOrigin::File ? PrCore::PathUtils::GetFile(p_resDesc->filePath) : p_resDesc->data->GetName();
		std::string path;
		if (!p_exportRoot.empty())
		{
			path = PrCore::PathUtils::MakePath(p_exportRoot, resourceName);
		}
		path += ".mat";

		if (PrSystems::Get<PrCore::FileSystem>()->FileExist(path))
			return;

		PrRenderer::MaterialLoader loader;
		auto success = loader.SaveResourceOnDisc(p_resDesc->data, path);
		if (!success)
		{
			PR_ASSERT(false, "Cannot export resource!");
			return;
		}

		p_resDesc->origin = ResourceOrigin::File;
		p_resDesc->state = ResourceState::Loaded;
		p_resDesc->filePath = path;
	}
}

void BasicConvexShapeExport(ResourceDescConstPtr p_resDesc, std::string_view p_exportRoot)
{
	if (p_resDesc->origin == ResourceOrigin::Memory)
	{
		auto resourceName = p_resDesc->origin == ResourceOrigin::File ? PrCore::PathUtils::GetFile(p_resDesc->filePath) : p_resDesc->data->GetName();
		std::string path;
		if (!p_exportRoot.empty())
		{
			path = PrCore::PathUtils::MakePath(p_exportRoot, resourceName);
		}
		path += ".physc";

		if (PrSystems::Get<PrCore::FileSystem>()->FileExist(path))
			return;

		PrPhysics::ConvexMeshLoader loader;
		auto success = loader.SaveResourceOnDisc(p_resDesc->data, path);
		if (!success)
		{
			PR_ASSERT(false, "Cannot export resource!");
			return;
		}

		p_resDesc->origin = ResourceOrigin::File;
		p_resDesc->state = ResourceState::Loaded;
		p_resDesc->filePath = path;
	}
}

void SceneExporter::SaveMemoryResourcesToFile(std::string_view p_exportRoot)
{
	auto meshExporter = [p_exportRoot](ResourceDescConstPtr p_resDesc)
	{
		BasicMeshExport(p_resDesc, p_exportRoot);
	};

	auto textureExporter = [p_exportRoot](ResourceDescConstPtr p_resDesc)
	{
		BasicTextureExport(p_resDesc, p_exportRoot);
	};

	auto materialExporter = [p_exportRoot](ResourceDescConstPtr p_resDesc)
	{
		BasicMaterialExport(p_resDesc, p_exportRoot);
	};

	auto convexShapeExporter = [p_exportRoot](ResourceDescConstPtr p_resDesc)
	{
		BasicConvexShapeExport(p_resDesc, p_exportRoot);
	};

	if (!PrSystems::Get<PrCore::FileSystem>()->FileExist(p_exportRoot))
	{
		PrSystems::Get<PrCore::FileSystem>()->CreateDir(p_exportRoot);
	}

	auto pResourceSystem = PrSystems::Get<PrCore::ResourceSystem>();
	pResourceSystem->ForEachResource<PrRenderer::Mesh>(meshExporter);
	pResourceSystem->ForEachResource<PrRenderer::Texture>(textureExporter);
	pResourceSystem->ForEachResource<PrPhysics::IConvexMesh>(convexShapeExporter);
	PrSystems::Get<PrCore::JobSystem>()->WaitAll();
	pResourceSystem->ForEachResource<PrRenderer::Material>(materialExporter);
}
