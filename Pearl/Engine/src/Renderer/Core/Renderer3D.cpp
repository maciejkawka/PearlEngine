#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/LowRenderer.h"
#include"Renderer/Core/Camera.h"

#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"  //Cannot debug with this line.
#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Core/Defines.h"
#include "Renderer/Buffers/Framebuffer.h"

using namespace PrRenderer::Core;

Renderer3D::Renderer3D()
{
	PrCore::Events::EventListener windowResizedListener;
	windowResizedListener.connect<&Renderer3D::OnWindowResize>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	m_color = PrCore::Math::vec3(0.0f);

	m_quad = Resources::Mesh::CreatePrimitive(Resources::Quad);
}

void Renderer3D::Begin()
{
	Core::LowRenderer::EnableDepth(true);
	Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
	LowRenderer::ClearColor(0.1f, 0.1f, 0.8f, 1.0f);
}

void Renderer3D::SetCubemap(PrRenderer::Resources::MaterialPtr p_cubemap)
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

void Renderer3D::AddLight(const PrCore::Math::mat4& p_lightmMat)
{
	if (m_lightData.size() == MAX_LIGHTNUM)
	{
		PRLOG_WARN("Renderer supports {0} lights only", MAX_LIGHTNUM);
		return;
	}

	m_lightData.push_back(p_lightmMat);
}

void Renderer3D::SetAmbientLight(PrRenderer::Core::Color p_ambientColor)
{
	m_color = p_ambientColor;
}

void Renderer3D::DrawMeshNow(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material)
{
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	const auto& VPMatrix = camera->RecalculateMatrices();
	auto modelMatrix = PrCore::Math::translate(PrCore::Math::mat4(1.0f), p_position);
	modelMatrix  = modelMatrix * PrCore::Math::mat4_cast(p_rotation);
	modelMatrix = PrCore::Math::scale(modelMatrix, p_scale);

	if(p_material->HasProperty("camPos"))
		p_material->SetProperty("camPos", camera->GetPosition());
	
	if (p_material->HasProperty("VPMatrix"))
		p_material->SetProperty("VPMatrix", VPMatrix);
	
	if (p_material->HasProperty("modelMatrix"))
		p_material->SetProperty("modelMatrix", modelMatrix);

	if (p_material->HasProperty("MVP"))
		p_material->SetProperty("MVP", VPMatrix * modelMatrix);
	
	if (p_material->HasProperty("ambientColor"))
		p_material->SetProperty("ambientColor", m_color);

	if (!m_lightData.empty())
	{
		if (p_material->HasProperty("lightMat[0]"))
			p_material->SetPropertyArray("lightMat[0]", m_lightData.data(), m_lightData.size());

		if (p_material->HasProperty("lightNumber"))
			p_material->SetProperty("lightNumber", (int)m_lightData.size());
	}

	p_material->Bind();
	p_mesh->Bind();

	LowRenderer::Draw(p_mesh->GetVertexArray());
	p_material->Unbind();
	p_mesh->Unbind();
}

void Renderer3D::Flush()
{
	if(m_cubemap)
		DrawCubemap();

	m_lightData.clear();
}

void Renderer3D::OnWindowResize(PrCore::Events::EventPtr p_event)
{
	auto windowResizeEvent = std::static_pointer_cast<PrCore::Events::WindowResizeEvent>(p_event);
	auto width = windowResizeEvent->m_width;
	auto height = windowResizeEvent->m_height;

	LowRenderer::SetViewport(width, height);
}

void Renderer3D::DrawCubemap()
{
	auto camera = PrRenderer::Core::Camera::GetMainCamera();
	if (m_cubemap->HasProperty("view"))
		m_cubemap->SetProperty("view", camera->GetViewMatrix());
	if (m_cubemap->HasProperty("proj"))
		m_cubemap->SetProperty("proj", camera->GetProjectionMatrix());

	LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::LessEqual);

	m_cubemap->Bind();
	m_quad->Bind();
	LowRenderer::Draw(m_quad->GetVertexArray(), Primitives::TriangleStrip);
	m_quad->Unbind();
	m_cubemap->Unbind();
	LowRenderer::SetDepthAlgorythm(ComparaisonAlgorithm::Less);
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

	FramebuffferPtr framebuffer = Buffers::Framebufffer::Create(settings);

	auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("IrradianceMap.shader");
	auto cube = PrRenderer::Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
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
		
		PrRenderer::Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
		Core::LowRenderer::Draw(cube->GetVertexArray());
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

	FramebuffferPtr framebuffer = Buffers::Framebufffer::Create(settings);

	auto shader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("prefilteredCube.shader");
	auto cube = PrRenderer::Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Cube);
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
			Core::LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
			Core::LowRenderer::Draw(cube->GetVertexArray());
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
	auto quad = PrRenderer::Resources::Mesh::CreatePrimitive(Resources::PrimitiveType::Quad);

	shader->Bind();
	quad->Bind();
	framebuffer->Bind();

	LowRenderer::Clear(Core::ClearFlag::ColorBuffer | Core::ClearFlag::DepthBuffer);
	Core::LowRenderer::Draw(quad->GetVertexArray());

	m_LUTMap = framebuffer->GetTexturePtr();

	shader->Unbind();
	quad->Unbind();
	framebuffer->Unbind();

	PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("LUTMap.shader");
}
