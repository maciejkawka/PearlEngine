#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/LowRenderer.h"
#include"Renderer/Core/Camera.h"

#include"Core/Events/WindowEvents.h"
#include"Core/Events/EventManager.h"  //Cannot debug with this line.
#include"Core/Resources/ResourceLoader.h"

#include "Renderer/Buffers/Framebuffer.h"
#include "Renderer/Core/BoundingVolume.h"

using namespace PrRenderer::Core;

Renderer3D::Renderer3D()
{
	PrCore::Events::EventListener windowResizedListener;
	windowResizedListener.connect<&Renderer3D::OnWindowResize>(this);
	PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	m_renderData.ambientColor = PrCore::Math::vec3(0.0f);

	m_opaqueObjects.reserve(MAX_OPAQUE_RENDERABLES);
	m_transparentObjects.reserve(MAX_TRANSPARENT_RENDERABLES);

	m_instancingShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("PBR/PBRwithIR_Instanced.shader");
	PR_ASSERT(m_instancingShader != nullptr, "Instance shader was not found");
}

void Renderer3D::Begin()
{
	LowRenderer::EnableCullFace(true);
	LowRenderer::EnableDepth(true);
	LowRenderer::Clear(Core::ClearFlag::ColorBuffer | ClearFlag::DepthBuffer);
	LowRenderer::ClearColor(0.1f, 0.1f, 0.8f, 1.0f);
}

void Renderer3D::SetCubemap(Resources::MaterialPtr p_cubemap)
{
	m_cubemap.reset();
	m_renderData.IRMap.reset();
	m_renderData.prefilterMap.reset();
	m_renderData.brdfLUT.reset();

	if (p_cubemap == nullptr)
	{
		auto blackTexture = Resources::Texture2D::GenerateBlackTexture();
		m_renderData.IRMap = std::static_pointer_cast<Resources::Cubemap>(blackTexture);
		m_renderData.prefilterMap = std::static_pointer_cast<Resources::Cubemap>(blackTexture);
		m_renderData.brdfLUT = blackTexture;
		m_renderData.hasCubeMap = false;
		return;
	}

	m_cubemap = p_cubemap;
	m_renderData.hasCubeMap = true;

	GenerateIRMap();
	GeneratePrefilterMap();
	GenerateLUTMap();
}

void Renderer3D::SetMainCamera(Camera* p_camera)
{
	m_renderData.mainCamera = p_camera;
	Camera::SetMainCamera(p_camera);
}

void Renderer3D::AddLight(const PrCore::Math::mat4& p_lightmMat)
{
	if (m_renderData.lightData.size() == MAX_LIGHTNUM)
	{
		PRLOG_WARN("Renderer supports {0} lights only", MAX_LIGHTNUM);
		return;
	}

	m_renderData.lightData.push_back(p_lightmMat);
}

void Renderer3D::SetAmbientLight(Color p_ambientColor)
{
	m_renderData.ambientColor = p_ambientColor;
}

void Renderer3D::AddMeshRenderObject(MeshRenderObject&& p_meshRenderObject)
{
	//Frustrum culling
	////Discard if out of frustrum
	const auto frustrum = Frustrum(m_renderData.mainCamera->GetProjectionMatrix(), m_renderData.mainCamera->GetViewMatrix());
	const auto bundingBox = BoxVolume(p_meshRenderObject.mesh->GetVertices());
	if (!bundingBox.IsOnFrustrum(frustrum, p_meshRenderObject.worldMat))
		return;

	//Calculate sorting hash
	std::uint32_t hashName = std::hash<std::string>{}(p_meshRenderObject.material->GetName());
	RenderSortingHash hash(p_meshRenderObject);
	hash.SetDepth(CalculateDepthValue(p_meshRenderObject.position));

	//Add objects to the list
	if(p_meshRenderObject.material->GetRenderType() == Resources::RenderType::Opaque)
	{
		if (m_opaqueObjects.size() >= m_opaqueObjects.capacity())
		{
			PRLOG_WARN("Renderer3D opaque buffer exceded the limit {0}, discarding object", m_opaqueObjects.capacity());
			return;
		}

		m_opaqueObjects.push_back(std::move(hash), p_meshRenderObject);
	}
	else
	{
		if (m_transparentObjects.size() >= m_transparentObjects.capacity())
		{
			PRLOG_WARN("Renderer3D tranparent buffer exceded the limit {0}, discarding object", m_transparentObjects.capacity());
			return;
		}

		m_transparentObjects.push_back(std::move(hash), p_meshRenderObject);
	}
}

void Renderer3D::DrawMesh(Resources::MeshPtr p_mesh, PrCore::Math::vec3 p_position, PrCore::Math::quat p_rotation, PrCore::Math::vec3 p_scale, Resources::MaterialPtr p_material)
{
	//Calculate modelMatrix
	auto position = p_position;
	auto rotation = p_rotation;
	auto scale = p_scale;
	auto modelMatrix = PrCore::Math::translate(PrCore::Math::mat4(1.0f), position);
	modelMatrix = modelMatrix * PrCore::Math::mat4_cast(rotation);
	modelMatrix *= PrCore::Math::scale(PrCore::Math::mat4(1.0f), scale);

	MeshRenderObject object{
		p_mesh,
		p_material,
		std::move(modelMatrix),
		std::move(position)
	};

	AddMeshRenderObject(std::move(object));
}

void Renderer3D::Render()
{
	//Render cubemap
	if(m_cubemap)
		m_RCQueue.push(new CubemapRenderRC(m_cubemap, &m_renderData));
	
	m_opaqueObjects.Sort();
	m_transparentObjects.Sort();

	//Schedule rendering
	if(m_useInstancing)
	{
		InstantateMeshObjects<decltype(m_opaqueObjects), true>(m_opaqueObjects);
		InstantateMeshObjects<decltype(m_transparentObjects), true>(m_transparentObjects);
	}
	//Render normal way
	else
	{
		for (auto [_, object] : m_opaqueObjects)
			m_RCQueue.push(new MeshRenderRC(std::move(*object), &m_renderData));
		for (auto [_, object] : m_transparentObjects)
			m_RCQueue.push(new MeshRenderRC(std::move(*object), &m_renderData));
	}
}

void Renderer3D::Flush()
{
	m_renderData.mainCamera->RecalculateMatrices();

	while (!m_RCQueue.empty())
	{
		auto command = m_RCQueue.front();
		command->Invoke();
		delete command;
		m_RCQueue.pop();
	}

	m_transparentObjects.clear();
	m_opaqueObjects.clear();
	m_renderData.lightData.clear();
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

	m_renderData.IRMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr(0));

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

	m_renderData.prefilterMap = std::static_pointer_cast<Resources::Cubemap>(framebuffer->GetTexturePtr());

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

	m_renderData.brdfLUT = framebuffer->GetTexturePtr();

	shader->Unbind();
	quad->Unbind();
	framebuffer->Unbind();

	PrCore::Resources::ResourceLoader::GetInstance().DeleteResource<Resources::Shader>("LUTMap.shader");
}

size_t Renderer3D::CalculateDepthValue(const PrCore::Math::vec3& p_position) const
{
	auto camera = m_renderData.mainCamera;
	auto distance = PrCore::Math::distance(p_position, camera->GetPosition());
	return (distance - camera->GetNear()) / (camera->GetFar() - camera->GetNear()) * 0xFFFFFF;
}
