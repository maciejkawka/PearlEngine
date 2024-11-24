#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Texture2DLoader.h"
#include "Renderer/Resources/Texture2D.h"
#include "Renderer/OpenGL/GLUtils.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Utils/PathUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stb/stb_image.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

PrCore::Resources::IResourceDataPtr Texture2DLoader::LoadResource(const std::string& p_path)
{
	int width = 0;
	int heigth = 0;
	int channelsNumber = 0;
	TextureFormat format = TextureFormat::None;


	// Change that in future
	std::string dir = PrCore::Filesystem::FileSystem::GetInstance().GetResourcesPath();
	std::string resourcePath = PrCore::PathUtils::MakePath(dir, TEXTURE_DIR);
	resourcePath = PrCore::PathUtils::MakePath(resourcePath, p_path);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(resourcePath.c_str(), &width, &heigth, &channelsNumber, 0);

	if (!data)
		return nullptr;

	//HDR texture
	if (PrCore::PathUtils::GetExtensionInPlace(resourcePath) == "hdr")
	{
		switch (channelsNumber)
		{
		case 3:
			format = Resources::TextureFormat::RGB16F;
			break;
		case 4:
			format = Resources::TextureFormat::RGBA16F;
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
			format = Resources::TextureFormat::R8;
			break;
		case 2:
			format = Resources::TextureFormat::RG16;
			break;
		case 3:
			format = Resources::TextureFormat::RGB24;
			break;
		case 4:
			format = Resources::TextureFormat::RGBA32;
			break;
		default:
			PRLOG_WARN("Cannot specify texture {0} channel format", p_path);
			break;
		}
	}

	// Create texture
	auto texture = Texture2D::Create();

	texture->SetFormat(format);
	texture->SetWidth(width);
	texture->SetHeight(heigth);
	texture->SetData(data);

	texture->SetReadable(false);
	texture->SetMipMap(true);

	texture->Apply();
	texture->SetData(nullptr);

	delete[] data;

	return texture;
}

IResourceDataPtr Texture2DLoader::LoadFromMemoryResource(const void* p_buffer, size_t p_size, int p_flags)
{
	int width = 0;
	int heigth = 0;
	int channelsNumber = 0;
	TextureFormat format = TextureFormat::RGBA32;

	int desiredChannels = 0;
	if (p_flags & FlagDesiredChannelsGrayScale)
		desiredChannels = 1;
	else if (p_flags & FlagDesiredChannelsRGB)
		desiredChannels = 3;
	else if (p_flags & FlagDesiredChannelsRGBA)
		desiredChannels = 4;

	unsigned char* data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(p_buffer), p_size, &width, &heigth, &channelsNumber, desiredChannels);
	if (!data)
		return nullptr;

	switch (channelsNumber)
	{
	case 1:
		format = Resources::TextureFormat::R8;
		break;
	case 2:
		format = Resources::TextureFormat::RG16;
		break;
	case 3:
		format = Resources::TextureFormat::RGB24;
		break;
	case 4:
		format = Resources::TextureFormat::RGBA32;
		break;
	default:
		format = Resources::TextureFormat::RGBA32;
	}

	// Create texture
	auto texture = Texture2D::Create();

	texture->SetFormat(format);
	texture->SetWidth(width);
	texture->SetHeight(heigth);
	texture->SetData(data);

	texture->SetReadable(false);
	texture->SetMipMap(true);

	texture->Apply();
	texture->SetData(nullptr);

	delete[] data;

	return texture;
}

void Texture2DLoader::UnloadResource(IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool Texture2DLoader::SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
