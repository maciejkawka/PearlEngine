#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/MaterialLoader.h"
#include "Renderer/Resources/Material.h"

#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Utils;

PrCore::Resources::IResourceDataPtr MaterialLoader::LoadResource(const std::string& p_path)
{
	auto file = PrCore::File::FileSystem::GetInstance().OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	auto json = JSON::json::parse(dataVector);

	MaterialPtr mat = std::make_shared<Material>(json);

	return mat;
}

void MaterialLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool MaterialLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
