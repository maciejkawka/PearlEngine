#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/Texture2DLoader.h"
#include "Renderer/Resources/Texture2D.h"
#include "Renderer/OpenGL/GLUtils.h"

#include "Core/File/FileSystem.h"
#include "Core/Utils/PathUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include"stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include"stb/stb_image_write.h"

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

Texture2DLoader::Texture2DLoader()
{
	stbi_set_flip_vertically_on_load(true);
	stbi_flip_vertically_on_write(true);
}

PrCore::Resources::IResourceDataPtr Texture2DLoader::LoadResource(const std::string& p_path)
{
	int width = 0;
	int heigth = 0;
	int channelsNumber = 0;
	TextureFormat format = TextureFormat::None;

	// Change that in future
	auto file = PrCore::File::FileSystem::GetInstance().OpenFileWrapper(p_path);
	if (file == nullptr)
		return nullptr;

	size_t fileSize = file->GetSize();
	unsigned char* buffer = new unsigned char[fileSize];
	file->Read(buffer, fileSize);

	unsigned char* data = stbi_load_from_memory(buffer, fileSize, &width, &heigth, &channelsNumber, 0);
	
	delete[] buffer;

	if (!data)
		return nullptr;

	//HDR texture
	if (PrCore::PathUtils::GetExtensionInPlace(p_path) == "hdr")
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

void TextureWriteCallback(void* context, void* data, int size)
{
	auto bufferPair = static_cast<std::pair<void*, int*>*>(context);
	byte* buffer = static_cast<byte*>(bufferPair->first);
	int* bufferSize = bufferPair->second;

	byte* bytes = static_cast<byte*>(data);
	for (int i = 0; i < size; i++)
	{
		buffer[*bufferSize] = bytes[i];
		++*bufferSize;
	}
}

bool Texture2DLoader::SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path)
{
	auto texturePtr = std::static_pointer_cast<Texture2D>(p_resourceData);

	int format = 0;
	switch (texturePtr->GetFormat())
	{
	case Resources::TextureFormat::R8:
		format = 1;
		break;
	case Resources::TextureFormat::RG16:
		format = 2;
		break;
	case Resources::TextureFormat::RGB24:
		format = 3;
		break;
	case Resources::TextureFormat::RGBA32:
		format = 4;
		break;
	default:
		format = 4;
	}

	void* rawData = texturePtr->FetchGPUData();
	void* compressedData = nullptr;
	int buffSize = 0;

	auto ext = PrCore::PathUtils::GetExtension(p_path);
	if (ext == ".png")
	{
		compressedData = stbi_write_png_to_mem(static_cast<const byte*>(rawData), texturePtr->GetWidth() * format, texturePtr->GetHeight(), texturePtr->GetWidth(), 4, &buffSize);
		if (compressedData == nullptr)
		{
			delete[]rawData;
			return false;
		}
	}
	else if (ext == ".tga")
	{
		compressedData = new byte[texturePtr->GetWidth() * texturePtr->GetHeight() * format + 64];
		auto bufferPair = std::make_pair(compressedData, &buffSize);

		if (!stbi_write_tga_to_func(TextureWriteCallback, &bufferPair, texturePtr->GetWidth(), texturePtr->GetHeight(), format, rawData))
		{
			delete[]rawData;
			delete[]compressedData;
			return false;
		}
	}
	else if (ext == ".jpg")
	{
		compressedData = new byte[texturePtr->GetWidth() * texturePtr->GetHeight() * format + 64];
		auto bufferPair = std::make_pair(compressedData, &buffSize);

		if (!stbi_write_jpg_to_func(TextureWriteCallback, &bufferPair, texturePtr->GetWidth(), texturePtr->GetHeight(), format, rawData, 100))
		{
			delete[]rawData;
			delete[]compressedData;
			return false;
		}
	}
	else
	{
		delete[]rawData;
		PR_ASSERT(false, "Texture extension not supported.");
		return false;
	}

	auto file = PrCore::File::FileSystem::GetInstance().FileOpen(p_path, PrCore::File::OpenMode::Write);
	if (file == nullptr)
	{
		delete[]rawData;
		delete[]compressedData;
		return false;
	}

	PrCore::File::FileSystem::GetInstance().FileWrite(file, compressedData, buffSize);
	PrCore::File::FileSystem::GetInstance().FileClose(file);

	delete[]rawData;
	delete[]compressedData;

	return true;
}
