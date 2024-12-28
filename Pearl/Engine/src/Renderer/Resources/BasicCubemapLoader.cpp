#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/BasicCubemapLoader.h"
#include "Renderer/Resources/Cubemap.h"
#include "Renderer/Resources/Texture2D.h"
#include "Renderer/OpenGL/GLUtils.h"

#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"
#include "Core/Utils/JSONParser.h"

#include"stb/stb_image.h"

using namespace PrRenderer::Resources;

IResourceDataPtr BasicCubemapLoader::LoadResource(const std::string& p_path)
{
	// Open cubemap asset file that contains 6 textures
	std::vector<std::string> facesTexPaths;

	auto file = PrCore::File::FileSystem::GetInstance().OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data, file->GetSize());

	std::vector<uint8_t> dataVector;
	for (auto i = 0; i < file->GetSize(); i++)
		dataVector.push_back(*(data + i));
	delete[] data;

	auto json = PrCore::Utils::JSON::json::parse(dataVector);

	facesTexPaths.push_back(json["right"]);
	facesTexPaths.push_back(json["left"]);
	facesTexPaths.push_back(json["top"]);
	facesTexPaths.push_back(json["bottom"]);
	facesTexPaths.push_back(json["front"]);
	facesTexPaths.push_back(json["back"]);

	//Load textures
	std::vector<unsigned char*> dataArray;

	int texWidth;
	int texHeigth;
	TextureFormat texFormat;

	auto texData = LoadTexture(facesTexPaths[0], texWidth, texHeigth, texFormat);
	if (texData == nullptr)
		return nullptr;
	dataArray.push_back(texData);

	for (int i = 1; i < 6; i++)
	{
		int width;
		int heigth;
		TextureFormat format;

		texData = LoadTexture(facesTexPaths[i], width, heigth, format);

		bool isCorrupted = texData == nullptr || width != texWidth || heigth != texHeigth || texFormat != format;
		if (isCorrupted)
		{
			// Delete all loaded textures
			for (auto data : dataArray)
				delete[] data;

			return nullptr;
		}

		dataArray.push_back(texData);
	}

	// All good create cubemap now

	auto cubemap = Cubemap::Create();

	cubemap->SetFormat(texFormat);
	cubemap->SetWidth(texWidth);
	cubemap->SetHeight(texHeigth);
	cubemap->SetData(dataArray.data());

	cubemap->SetReadable(false);
	cubemap->SetMipMap(true);

	cubemap->Apply();
	cubemap->SetData(nullptr);

	// Delete loaded texture
	for (auto data : dataArray)
		delete[] data;

	return cubemap;
}

unsigned char* BasicCubemapLoader::LoadTexture(const std::string& p_path, int& p_width, int& p_height, Resources::TextureFormat& p_format)
{
	// Load textures
	int channelsNumber = 0;

	auto file = PrCore::File::FileSystem::GetInstance().OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	size_t fileSize = file->GetSize();
	unsigned char* data = new unsigned char[fileSize];
	file->Read(data, fileSize);

	stbi_set_flip_vertically_on_load(false);
	unsigned char* texData = stbi_load_from_memory(data, fileSize, &p_width, &p_height, &channelsNumber, 0);

	delete [] data;
	if (!texData)
		return nullptr;

	//HDR texture
	if (PrCore::PathUtils::GetExtensionInPlace(p_path) == "hdr")
	{
		switch (channelsNumber)
		{
		case 3:
			p_format = Resources::TextureFormat::RGB16F;
			break;
		case 4:
			p_format = Resources::TextureFormat::RGBA16F;
			break;
		default:
			PRLOG_WARN("Cannot specify texture {0} channel format", p_path);
			break;
		}
	}
	// Normal texture
	else
	{
		switch (channelsNumber)
		{
		case 1:
			p_format = Resources::TextureFormat::R8;
			break;
		case 2:
			p_format = Resources::TextureFormat::RG16;
			break;
		case 3:
			p_format = Resources::TextureFormat::RGB24;
			break;
		case 4:
			p_format = Resources::TextureFormat::RGBA32;
			break;
		default:
			PRLOG_WARN("Cannot specify texture {0} channel format", p_path);
			break;
		}
	}

	return texData;
}

bool BasicCubemapLoader::SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}

void BasicCubemapLoader::UnloadResource(IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}
