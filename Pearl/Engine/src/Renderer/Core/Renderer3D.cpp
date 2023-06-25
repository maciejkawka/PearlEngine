#include"Core/Common/pearl_pch.h"

#include"Renderer/Core/Renderer3D.h"
#include"Renderer/Core/LowRenderer.h"
#include"Renderer/Core/Camera.h"

#include"Core/Events/WindowEvents.h"
//#include"Core/Events/EventManager.h"  //Cannot debug with this line.
#include"Core/Resources/ResourceLoader.h"

#include "Renderer/Buffers/Framebuffer.h"
#include "Renderer/Core/BoundingVolume.h"

using namespace PrRenderer::Core;

Renderer3D::Renderer3D()
{
	//PrCore::Events::EventListener windowResizedListener;
	//windowResizedListener.connect<&Renderer3D::OnWindowResize>(this);
	//PrCore::Events::EventManager::GetInstance().AddListener(windowResizedListener, PrCore::Events::WindowResizeEvent::s_type);
	m_color = PrCore::Math::vec3(0.0f);


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
	//Frustrum culling
	////Discard if out of frustrum
	const auto frustrum = Frustrum(m_mainCamera->GetProjectionMatrix(), m_mainCamera->GetViewMatrix());
	const auto bundingBox = BoxVolume(p_meshRenderObject.mesh->GetVertices());
	if (!bundingBox.IsOnFrustrum(frustrum, p_meshRenderObject.worldMat))
		return;

	//

	//Calculate sorting hash
	std::uint32_t hashName = std::hash<std::string>{}(p_meshRenderObject.material->GetName());
	RenderSortingHash hash(p_meshRenderObject);
	hash.SetDepth(CalculateDepthValue(p_meshRenderObject.position));
	//

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
	m_opaqueObjects.Sort();

	//Try instancing
	CreateInstancesOpaqueMesh();

	//Normal render
	for (auto [_, object] : m_opaqueObjects)
	{
		if(object != nullptr)
		{
			//PRLOG_INFO("RENDERING {0}", object->mesh->GetName());
			m_RCQueue.push(new MeshRenderRC(std::move(*object), renderData));
		}
	}

	//Render Transparent
	m_transparentObjects.Sort();
	for (auto [_, object] : m_transparentObjects)
	{
		m_RCQueue.push(new TransparentMeshRenderRC(std::move(*object), renderData));
	}
	//PRLOG_INFO("RENDERED FINISHED");
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

	m_transparentObjects.clear();
	m_opaqueObjects.clear();
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

//void Renderer3D::CreateInstancesOpaqueMesh()
//{
//	for (auto it = m_opaqueMeshPriority.begin(); it <= m_opaqueMeshPriority.end(); ++it)
//	{
//		auto iterator = it->second;
//		auto prorityHash = it->first;
//		auto materialHash = prorityHash.GetMaterialHash();
//		auto renderOrder = prorityHash.GetRenderOrder();
//
//		//Find candidates to instance
//		size_t instanceCount = 0;
//		std::vector<MeshObjectPriority::iterator> m_instancedObjects;
//		auto localIt = it;
//		do
//		{
//			auto candidateMatHash = localIt->first;
//			if (candidateMatHash.GetMaterialHash() == materialHash && candidateMatHash.GetRenderOrder() == renderOrder)
//			{
//				m_instancedObjects.push_back(localIt);
//				instanceCount++;
//			}
//
//			++localIt;
//		} while (localIt->first.GetMaterialHash() == materialHash);
//
//		//If worth instancing
//		if (instanceCount >= MIN_INSTANCE_COUNT)
//		{
//			auto instancedShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("PBR/PBRwithIR_Instanced.shader");
//			PR_ASSERT(instancedShader != nullptr, "Instance shader was not found");
//
//			for (int i = 0; i < instanceCount; )
//			{
//				//Grab all data
//				Resources::MeshPtr mesh = m_instancedObjects[0]->second->mesh;
//				Resources::MaterialPtr material = m_instancedObjects[0]->second->material;
//				Resources::MaterialPtr instancedMaterial = std::make_shared<Resources::Material>(instancedShader);
//				instancedMaterial->CopyPropertiesFrom(*material);
//
//				std::vector<PrCore::Math::mat4> matrices;
//				for (auto object : m_instancedObjects)
//				{
//					matrices.push_back(std::move(object->second->worldMat));
//					m_opaqueMeshPriority.erase(object);
//					i++;
//				}
//
//				RenderData renderData{
//					m_lightData,
//					m_IRMap,
//					m_prefilteredMap,
//					m_LUTMap,
//					m_color,
//					false
//				};
//
//				InstancedMeshObject mehsObject{
//					mesh,
//					instancedMaterial,
//					instanceCount,
//					std::move(matrices)
//				};
//
//				m_RCQueue.push(new InstancedMeshesRC(std::move(mehsObject), renderData));
//			}
//		}
//	}
//}

size_t Renderer3D::CalculateDepthValue(const PrCore::Math::vec3& p_position)
{
	auto distance = PrCore::Math::distance(p_position, m_mainCamera->GetPosition());
	return (distance - m_mainCamera->GetNear()) / (m_mainCamera->GetFar() - m_mainCamera->GetNear());
}
