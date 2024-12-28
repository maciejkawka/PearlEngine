#include "Core/Common/pearl_pch.h"

#include "Renderer/Resources/HdrCubemapLoader.h"

#include "Core/Resources/ResourceSystem.h"
#include "Renderer/Resources/Texture2D.h"
#include "Renderer/Resources/Cubemap.h"
#include "Renderer/Resources/Shader.h"
#include "Renderer/Resources/Mesh.h"
#include "Renderer/Buffers/Framebuffer.h"

#include <Renderer/Core/LowRenderer.h>

using namespace PrRenderer::Resources;
using namespace PrCore::Resources;

PrCore::Resources::IResourceDataPtr HdrCubemapLoader::LoadResource(const std::string& p_path)
{
	//Load HDR texture and shader
	auto textureResource = ResourceSystem::GetInstance().Load<Texture>(p_path);
	auto texture = textureResource.GetData();
	auto shader = ResourceSystem::GetInstance().Load<Shader>("shader/cubemap/hdr_to_cubemap.shader").GetData();

	//Create Framebuffer 
	Buffers::FramebufferSettings fbSettings;
	fbSettings.globalHeight = texture->GetWidth();
	fbSettings.globalWidth = texture->GetWidth();
;
	Buffers::FramebufferTexture fbTex;
	fbTex.cubeTexture = true;
	fbTex.format = Resources::TextureFormat::RGB24;
	fbTex.filteringMag = Resources::TextureFiltering::Nearest;
	fbTex.filteringMin = Resources::TextureFiltering::Nearest;

	fbSettings.colorTextureAttachments.textures.push_back(fbTex);

	auto framebuffer = Buffers::Framebufffer::Create(fbSettings);

	//Convert HDR to cubemap
	PrCore::Math::mat4 captureProjection = PrCore::Math::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	PrCore::Math::mat4 captureViews[] =
	{
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		PrCore::Math::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	shader->Bind();
	shader->SetUniformMat4("projection", captureProjection);
	shader->SetUniformInt("equirectangularMap", 0);
	texture->Bind(0);

	framebuffer->Bind();

	auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
	cube->Bind();
	for (int i = 0; i < 6; i++)
	{
		framebuffer->SetAttachmentDetails(0, i);
		shader->SetUniformMat4("view", captureViews[i]);

		Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
		Core::LowRenderer::Draw(cube->GetVertexArray());
	}

	shader->Unbind();
	texture->Unbind();
	framebuffer->Unbind();
	cube->Unbind();

	return std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr());
}

void HdrCubemapLoader::UnloadResource(PrCore::Resources::IResourceDataPtr p_resourceData)
{
	p_resourceData.reset();
	p_resourceData = nullptr;
}

bool HdrCubemapLoader::SaveResourceOnDisc(PrCore::Resources::IResourceDataPtr p_resourceData, const std::string& p_path)
{
	return false;
}
