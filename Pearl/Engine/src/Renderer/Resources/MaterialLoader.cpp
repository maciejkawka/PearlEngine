#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/MaterialLoader.h"
#include "Renderer/Resources/Material.h"

#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"

using namespace PrRenderer;
using namespace PrCore::Utils;

PrCore::IResourceDataPtr MaterialLoader::LoadResource(const std::string& p_path)
{
	auto file = PrSystems::Get<PrCore::FileSystem>()->OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());

	std::vector<uint8_t> dataVector;

	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	auto json = JSON::json::parse(dataVector);

	MaterialPtr mat = std::make_shared<Material>();
	mat->OnDeserialize(json);
	return mat;
}

void MaterialLoader::UnloadResource(PrCore::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool MaterialLoader::SaveResourceOnDisc(PrCore::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	MaterialPtr materialPtr = std::static_pointer_cast<Material>(p_resourceData);

	PrCore::Utils::JSON::json serialize;
	materialPtr->OnSerialize(serialize);

	auto serializeStr = serialize.dump(4);

	auto pFileSystem = PrSystems::Get<PrCore::FileSystem>();
	auto file = pFileSystem->FileOpen(p_path, PrCore::FileOpenMode::Write);
	pFileSystem->FileWrite(file, serializeStr.c_str(), serializeStr.length());
	pFileSystem->FileClose(file);

	return true;
}
