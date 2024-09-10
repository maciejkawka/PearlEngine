#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/CubemapLoader.h"
#include "Renderer/Resources/Cubemapv2.h"
#include "Renderer/Resources/Texture2Dv2.h"
#include "Renderer/OpenGL/GLUtils.h"

#include "Core/Filesystem/FileSystem.h"
#include "Core/Utils/PathUtils.h"
#include "Core/Utils/JSONParser.h"

#include"stb/stb_image.h"

using namespace PrRenderer::Resources;

IResourceDataPtr CubemapLoader::LoadResource(const std::string& p_path)
{
	IResourceDataPtr resource = nullptr;
	if (PrCore::PathUtils::GetExtensionInPlace(p_path) == "hdr")
		resource = LoadHDR(p_path);
	else
		resource = LoadSixSided(p_path);


	return resource;
}

PrCore::Resources::IResourceDataPtr CubemapLoader::LoadHDR(const std::string& p_path)
{
	// Implement later when shader and resource is ready
	//  
	////Load HDR texture and shader
	//Texture2Dv2Ptr texture = nullptr;
	// shader = nullptr;
	//	//PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Texture2D>(m_name);
	//	//PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Cubemap/HDRToCubemap.shader");

	////Create Framebuffer 
	//Buffers::FramebufferSettings fbSettings;
	//fbSettings.globalHeight = texture->GetWidth();
	//fbSettings.globalWidth = texture->GetWidth();

	////fbSettings.depthStencilAttachment = { Resources::TextureFormat::Depth24 };

	//Buffers::FramebufferTexture fbTex;
	//fbTex.cubeTexture = true;
	//fbTex.format = Resources::TextureFormat::RGB24;
	//fbTex.filteringMag = Resources::TextureFiltering::Nearest;
	//fbTex.filteringMin = Resources::TextureFiltering::Nearest;

	//fbSettings.colorTextureAttachments.textures.push_back(fbTex);

	//auto framebuffer = Buffers::Framebufffer::Create(fbSettings);

	////Convert HDR to cubemap
	//PrCore::Math::mat4 captureProjection = PrCore::Math::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	//PrCore::Math::mat4 captureViews[] =
	//{
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
	//	PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	//};

	//shader->Bind();
	//shader->SetUniformMat4("projection", captureProjection);
	//shader->SetUniformInt("equirectangularMap", 0);
	//texture->Bind(0);

	//framebuffer->Bind();

	//auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
	//cube->Bind();
	//for (int i = 0; i < 6; i++)
	//{
	//	framebuffer->SetAttachmentDetails(0, i);
	//	shader->SetUniformMat4("view", captureViews[i]);

	//	Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);

	//	Core::LowRenderer::Draw(cube->GetVertexArray());
	//}

	//shader->Unbind();
	//texture->Unbind();
	//framebuffer->Unbind();
	//cube->Unbind();

	//PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Texture2D>(m_name);
	//PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("HDRToCubemap.shader");

	////Set cubemap
	//m_ID = framebuffer->GetTextureID();

	//return true;

	return nullptr;
}

PrCore::Resources::IResourceDataPtr CubemapLoader::LoadSixSided(const std::string& p_path)
{
	// Open cubemap asset file that contains 6 textures
	std::vector<std::string> facesTexPaths;

	// Change that in the future
	std::string resourcePath = PrCore::PathUtils::MakePath(TEXTURE_DIR, p_path);
	PrCore::Filesystem::FileStreamPtr file = PrCore::Filesystem::FileSystem::GetInstance().OpenFileStream(resourcePath.c_str());
	if (file == nullptr)
		return nullptr;

	char* data = new char[file->GetSize()];
	file->Read(data);

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

	auto cubemap = Cubemapv2::Create();

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

unsigned char* CubemapLoader::LoadTexture(const std::string& p_path, int& p_width, int& p_height, Resources::TextureFormat& p_format)
{
	// Load textures
	int channelsNumber = 0;

	std::string dir = PrCore::Filesystem::FileSystem::GetInstance().GetResourcesPath();
	std::string resourcePath = PrCore::PathUtils::MakePath(dir, TEXTURE_DIR);
	resourcePath = PrCore::PathUtils::MakePath(resourcePath, p_path);

	stbi_set_flip_vertically_on_load(true);
	unsigned char* texData = stbi_load(resourcePath.c_str(), &p_width, &p_height, &channelsNumber, 0);

	if (!texData)
		return nullptr;

	//HDR texture
	if (PrCore::PathUtils::GetExtensionInPlace(resourcePath) == "hdr")
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

bool CubemapLoader::SaveResourceOnDisc(IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}

void CubemapLoader::UnloadResource(IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}
