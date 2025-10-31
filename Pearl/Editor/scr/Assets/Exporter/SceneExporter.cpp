#include "Editor/Assets/Exporter/SceneExporter.h"

#include "Core/Resources/ResourceSystem.h"
#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"

#include "Renderer/Resources/Mesh.h"
#include "Renderer/Resources/MeshOBJLoader.h"
#include "Renderer/Resources/Texture.h"
#include "Renderer/Resources/Texture2DLoader.h"
#include "Renderer/Resources/Material.h"
#include "Renderer/Resources/MaterialLoader.h"

using namespace PrEditor::Assets;
using namespace PrCore::Resources;

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

			if (PrCore::File::FileSystem::GetInstance().FileExist(path))
				return;

			PrRenderer::Resources::MeshOBJLoader loader;
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

		PrCore::Threading::JobSystem::GetInstance().Schedule("Export Mesh", lambda);
	}
}

void BasicTextureExport(ResourceDescConstPtr p_resDesc, std::string_view p_exportRoot)
{
	if (p_resDesc->origin == ResourceOrigin::Memory)
	{
		auto texturePtr = std::static_pointer_cast<PrRenderer::Resources::Texture>(p_resDesc->data);
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

			if (PrCore::File::FileSystem::GetInstance().FileExist(path))
				return;

			PrRenderer::Resources::Texture2DLoader loader;
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

		PrCore::Threading::JobSystem::GetInstance().Schedule("Export Texture", lambda);
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

		if (PrCore::File::FileSystem::GetInstance().FileExist(path))
			return;

		PrRenderer::Resources::MaterialLoader loader;
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

	if (!PrCore::File::FileSystem::GetInstance().FileExist(p_exportRoot))
	{
		PrCore::File::FileSystem::GetInstance().CreateDir(p_exportRoot);
	}

	PrCore::Resources::ResourceSystem::GetInstance().ForEachResource<PrRenderer::Resources::Mesh>(meshExporter);
	PrCore::Resources::ResourceSystem::GetInstance().ForEachResource<PrRenderer::Resources::Texture>(textureExporter);
	PrCore::Threading::JobSystem::GetInstance().WaitAll();
	PrCore::Resources::ResourceSystem::GetInstance().ForEachResource<PrRenderer::Resources::Material>(materialExporter);
}
