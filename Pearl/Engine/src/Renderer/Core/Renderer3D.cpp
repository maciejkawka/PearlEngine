#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/LowRenderer.h"
#include"Renderer/Core/Camera.h"

#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"  //Cannot debug with this line.
#include"Core/Resources/ResourceLoader.h"

#include "Renderer/Buffers/Framebuffer.h"

using namespace PrRenderer::Core;

Renderer3D::Renderer3D()
{
	PrCore::Events::EventListener windowResizedListener;
	windowResizedListener.connect<&Renderer3D::OnWindowResize>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	m_color = PrCore::Math::vec3(0.0f);


	m_transparentMeshObjects.reserve(MAX_OPAQUE_RENDERABLES);
	m_opaqueMeshObjects.reserve(MAX_TRANSPARENT_RENDERABLES);
}

void Renderer3D::Begin()
{
	LowRenderer::EnableDepth(true);
	LowRenderer::Clear(Core::ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
	LowRenderer::ClearColor(0.1f, 0.1f, 0.8f, 1.0f);
}

void Renderer3D::SetCubemap(Resources::MaterialPtr p_cubemap)
{
	m_cubemap.reset();
	m_IRMap.reset();
	m_prefilteredMap.reset();
	m_LUTMap.reset();

	if (p_cubemap == nullptr)
	{
		auto blackTexture = Resources::Texture2D::GenerateBlackTexture();
		m_IRMap = std::static_pointer_cast<Resources::Cubemap>(blackTexture);
		m_prefilteredMap = std::static_pointer_cast<Resources::Cubemap>(blackTexture);
		m_LUTMap = blackTexture;
		return;
	}

	m_cubemap = p_cubemap;

	GenerateIRMap();
	GeneratePrefilterMap();
	GenerateLUTMap();
}

void Renderer3D::SetMainCamera(Camera* p_camera)
{
	m_mainCamera = p_camera;
	Camera::SetMainCamera(p_camera);
}

void Renderer3D::AddLight(const PrCore::Math::mat4& p_lightmMat)
{
	if (m_lightData.size() == MAX_LIGHTNUM)
	{
		PRLOG_WARN("Renderer supports {0} lights only", MAX_LIGHTNUM);
		return;
	}

	m_lightData.push_back(p_lightmMat);
}

void Renderer3D::SetAmbientLight(Color p_ambientColor)
{
	m_color = p_ambientColor;
}

void Renderer3D::AddMeshRenderObject(MeshRenderObject&& p_meshRenderObject)
{
	//Calculate sorting hash
	std::uint32_t hashName = std::hash<std::string>{}(p_meshRenderObject.material->GetName());

	if(!p_meshRenderObject.isTransparent)
	{
		if (m_opaqueMeshObjects.size() >= m_opaqueMeshObjects.capacity())
		{
			PRLOG_WARN("Renderer3D opaque buffer exceded the limit {0}, discarding object", m_opaqueMeshObjects.capacity());
			return;
		}

		m_opaqueMeshObjects.push_back( p_meshRenderObject);
		m_opaqueMeshPriority.push_back({ hashName, m_opaqueMeshObjects.end() - 1 });
	}
	else
	{
		if (m_transparentMeshObjects.size() >= m_transparentMeshObjects.capacity())
		{
			PRLOG_WARN("Renderer3D tranparent buffer exceded the limit {0}, discarding object", m_transparentMeshObjects.capacity());
			return;
		}

		m_transparentMeshObjects.push_back(p_meshRenderObject);
		m_transparentMeshPriority.push_back({ hashName, m_transparentMeshObjects.end() - 1 });
	}
}

void Renderer3D::DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material)
{
	//TODO
}

void Renderer3D::Render()
{
	RenderData renderData{
		m_lightData,
		m_IRMap,
		m_prefilteredMap,
		m_LUTMap,
		m_color,
		false
	};

	if (m_prefilteredMap && m_IRMap && m_LUTMap != nullptr)
		renderData.hasCubeMap = true;

	//Render cubemap
	if(m_cubemap)
		m_RCQueue.push(new CubemapRenderRC(m_cubemap));


	//Render Opaque
	std::sort(m_opaqueMeshPriority.begin(), m_opaqueMeshPriority.end());
	for (auto& [_, object] : m_opaqueMeshPriority)
	{
		m_RCQueue.push(new MeshRenderRC(std::move(*object), renderData));
	}

	//Render Transparent
	std::sort(m_transparentMeshPriority.begin(), m_transparentMeshPriority.end());
	for (auto& [_, object] : m_transparentMeshPriority)
	{
		m_RCQueue.push(new MeshRenderRC(std::move(*object), renderData));
	}

}

void Renderer3D::Flush()
{
	m_mainCamera->RecalculateMatrices();

	while (!m_RCQueue.empty())
	{
		auto command = m_RCQueue.front();
		command->Invoke(m_mainCamera);
		delete command;
		m_RCQueue.pop();
	}

	m_transparentMeshPriority.clear();
	m_opaqueMeshPriority.clear();
	m_opaqueMeshObjects.clear();
	m_transparentMeshObjects.clear();
	m_lightData.clear();
}

void Renderer3D::OnWindowResize(PrCore::Events::EventPtr p_event)
{
	auto windowResizeEvent = std::static_pointer_cast<PrCore::Events::WindowResizeEvent>(p_event);
	auto width = windowResizeEvent->m_width;
	auto height = windowResizeEvent->m_height;
	
	LowRenderer::SetViewport(width, height);
}

void Renderer3D::GenerateIRMap()
{
	Buffers::FramebufferTexture texture;
	texture.format = Resources::TextureFormat::RGB16F;
	texture.cubeTexture = true;
	
	Buffers::FramebufferSettings settings;
	settings.globalWidth = 32;
	settings.globalHeight = 32;
	settings.colorTextureAttachments = texture;

	auto framebuffer = Buffers::Framebufffer::Create(settings);

	auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("IrradianceMap.shader");
	auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
	auto cubemap = m_cubemap->GetTexture("skybox");

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
	shader->SetUniformInt("environmentMap", 0);
	shader->SetUniformMat4("projection", captureProjection);
	cubemap->Bind();
	framebuffer->Bind();
	cube->Bind();

	for (int i = 0; i < 6; i++)
	{
		shader->SetUniformMat4("view", captureViews[i]);
		framebuffer->SetAttachmentDetails(0, i);
		
		LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
		LowRenderer::Draw(cube->GetVertexArray());
	}

	m_IRMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr(0));

	shader->Unbind();
	cubemap->Unbind();
	shader->Unbind();
	cube->Unbind();
	framebuffer->Unbind();

	PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("IrradianceMap.shader");
}

void Renderer3D::GeneratePrefilterMap()
{
	Buffers::FramebufferTexture texture;
	texture.format = Resources::TextureFormat::RGB16F;
	texture.filteringMin = Resources::TextureFiltering::LinearMipMapLinear;
	texture.filteringMag = Resources::TextureFiltering::Linear;
	texture.cubeTexture = true;

	Buffers::FramebufferSettings settings;
	settings.globalWidth = 128;
	settings.globalHeight = 128;
	settings.mipMaped = true;
	settings.colorTextureAttachments = texture;

	auto framebuffer = Buffers::Framebufffer::Create(settings);

	auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("prefilteredCube.shader");
	auto cube = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
	auto cubemap = m_cubemap->GetTexture("skybox");

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
	shader->SetUniformInt("environmentMap", 0);
	shader->SetUniformMat4("projection", captureProjection);
	cubemap->Bind();
	framebuffer->Bind();
	cube->Bind();

	unsigned int mipMapNumber = 5;
	for (int i = 0; i < mipMapNumber; i++)
	{
		float roughness = (float)i / (float)(mipMapNumber - 1);
		shader->SetUniformFloat("roughness", roughness);
		for (int j = 0; j < 6; j++)
		{
			shader->SetUniformMat4("view", captureViews[j]);
			framebuffer->SetAttachmentDetails(0, j, i);
			LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
			LowRenderer::Draw(cube->GetVertexArray());
		}
	}

	m_prefilteredMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr());

	shader->Unbind();
	cubemap->Unbind();
	shader->Unbind();
	cube->Unbind();
	framebuffer->Unbind();

	PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>(shader->GetName());
}

void Renderer3D::GenerateLUTMap()
{
	Buffers::FramebufferTexture texture;
	texture.format = Resources::TextureFormat::RG16;

	Buffers::FramebufferSettings settings;
	settings.globalHeight = 512;
	settings.globalWidth = 512;
	settings.colorTextureAttachments = texture;

	auto framebuffer = Buffers::Framebufffer::Create(settings);

	auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("LUTMap.shader");
	auto quad = Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Quad);

	shader->Bind();
	quad->Bind();
	framebuffer->Bind();

	LowRenderer::Clear(ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
	LowRenderer::Draw(quad->GetVertexArray());

	m_LUTMap = framebuffer->GetTexturePtr();

	shader->Unbind();
	quad->Unbind();
	framebuffer->Unbind();

	PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("LUTMap.shader");
}
