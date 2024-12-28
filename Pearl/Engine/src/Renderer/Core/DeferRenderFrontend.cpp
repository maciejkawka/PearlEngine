#include "Core/Common/pearl_pch.h"

#include "Renderer/Core/DeferRenderFrontend.h"

#include"Core/ECS/ECS.h"
#include"Core/Utils/Clock.h"

#include "Renderer/Core/DeferRenderBackend.h"
#include "Renderer/Core/BoundingVolume.h"

using namespace PrRenderer::Core;

#define FRAME_DATA_COUNT 2
#define MAX_INSTANCE_COUNT 200
#define MIN_INSTANCE_COUNT 5

static std::uint64_t m_frameID;

DeferRenderFrontend::DeferRenderFrontend(RendererSettings& p_settings) :
	IRenderFrontend(p_settings)
{
	//Prepare frame data
	m_currentFrame = m_frameData[0];
	m_previousFrame = m_frameData[1];
	m_currentFrameIndex = 0;

	m_nextPointLightPos = 0;
	m_nextSpotLightPos = 0;
	m_nextDirLightPos = 0;

	m_debugShader = PrCore::Resources::ResourceSystem::GetInstance().Load<Resources::Shader>("shader/debug_pass.shader").GetData();
	m_debugMaterial = std::make_shared<Resources::Material>(m_debugShader);

	m_rendererBackend = std::make_shared<DeferRenderBackend>(m_renderSettings);
}

void DeferRenderFrontend::SubmitLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id)
{
	// If Main light
	if(p_lightComponent->mainDirectLight)
	{
		if (m_currentFrame->mainDirectLight)
			PR_ASSERT(false, "FrontendRenderer: Main Direct light already added, cannot have more than one main lights in the scene");

		auto lightObject = std::make_shared<DirLightObject>();
		lightObject->id = p_id;
		lightObject->packedMat = p_lightComponent->m_light->CreatePackedMatrix(p_transformComponent->GetPosition(), -p_transformComponent->GetForwardVector());
		lightObject->shadowMapPos = SIZE_MAX; // ShadowMapPos is not important in main light
		lightObject->castShadow = p_lightComponent->m_shadowCast;
		m_currentFrame->mainDirectLight = lightObject;
		return;
	}

	const auto& light = p_lightComponent->m_light;
	LightObjectPtr lightObject = nullptr;
	switch (light->GetType())
	{
	case Resources::LightType::Directional:
	{
		if(m_dirLightNumber > MAX_LIGHT_DIRECT_COUNT)
		{
			PRLOG_WARN("FrontendRenderer: Discarding directional light, max limit {0} exceeded", MAX_LIGHT_DIRECT_COUNT);
			return;
		}

		lightObject = std::make_shared<DirLightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), -p_transformComponent->GetForwardVector());
		lightObject->id = p_id;
		m_dirLightNumber++;

		if (p_lightComponent->m_shadowCast && m_dirLightShadowNumber < m_renderSettings->dirLightMaxShadows)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextDirLightPos;
			m_nextDirLightPos++;
			m_dirLightShadowNumber++;
		}
		break;
	}
	case Resources::LightType::Point:
	{
		if (m_pointLightNumber > MAX_LIGHT_POINT_COUNT)
		{
			PRLOG_WARN("FrontendRenderer: Discarding point light, max limit {0} exceeded", MAX_LIGHT_POINT_COUNT);
			return;
		}

		lightObject = std::make_shared<LightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), -p_transformComponent->GetForwardVector());
		lightObject->id = p_id;
		m_pointLightNumber++;

		if (p_lightComponent->m_shadowCast && m_pointLightShadowNumber < m_renderSettings->pointLightMaxShadows)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextPointLightPos;
			m_nextPointLightPos += 6;
			m_pointLightShadowNumber++;
		}
		break;
	}
	case Resources::LightType::Spot:
	{
		if (m_spotLightNumber > MAX_LIGHT_SPOT_COUNT)
		{
			PRLOG_WARN("FrontendRenderer: Discarding spot light, max limit {0} exceeded", MAX_LIGHT_SPOT_COUNT);
			return;
		}

		lightObject = std::make_shared<SpotLightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), -p_transformComponent->GetForwardVector());
		lightObject->id = p_id;
		m_spotLightNumber++;

		if (p_lightComponent->m_shadowCast && m_spotLightShadowNumber < m_renderSettings->spotLightMaxShadows)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextSpotLightPos;
			m_nextSpotLightPos++;
			m_spotLightShadowNumber++;
		}
		break;
	}
	default:
	{
		PR_ASSERT(false, "Light type is invalid");
		break;
	}
	}

	m_currentFrame->lights.push_back(std::move(lightObject));
}

void DeferRenderFrontend::SubmitMesh(ECS::Entity& p_entity)
{
	PR_ASSERT(p_entity.HasComponent<ECS::MeshRendererComponent>(), "FrontendRenderer: entity does not have a MeshRendererComponent");
	PR_ASSERT(p_entity.HasComponent<ECS::TransformComponent>(), "FrontendRenderer: entity does not have a TransformComponent");

	auto meshComponent = p_entity.GetComponent<ECS::MeshRendererComponent>();
	auto mesh = meshComponent->mesh;
	if (mesh == nullptr)
		return;

	auto transformComponent = p_entity.GetComponent<ECS::TransformComponent>();
	auto worldMatrix = transformComponent->GetWorldMatrix();
	auto shadowMesh = meshComponent->shadowMesh;
	bool shouldCull = !mesh->GetBoxVolume().IsOnFrustrum(m_frustrum, worldMatrix);

	// Create RenderObject for each submesh
	for (int i = 0; i < mesh->GetSubmeshCount() && i < meshComponent->materials.size(); ++i)
	{
		auto material = meshComponent->materials[i];
		if (material == nullptr)
			continue;

		// Create renderObject
		RenderObjectPtr object = std::make_shared<RenderObject>();
		object->id = p_entity.GetID().GetID();
		object->material = material.GetData();
		object->vertexArrayPtr = mesh->GetVertexArray();
		object->vertexArrayShadowPtr = shadowMesh != nullptr ? shadowMesh->GetVertexArray() : nullptr;
		object->subMesh = mesh->GetSubmeshCount() > 0 ? mesh->GetSubmesh(i): Resources::SubMesh();
		object->boxVolume = mesh->GetBoxVolume();
		object->type = RenderObjectType::Mesh;
		object->worldMat = worldMatrix;

		// Calculate sorting hash
		SortingHash hash(*object);
		hash.SetDepth(RenderUtils::CalculateDepthValue(transformComponent->GetPosition(), m_camera));
		object->sortingHash = hash;

		// Add to shadow casters
		if (meshComponent->shadowCaster && material->GetRenderType() == Resources::RenderType::Opaque)
			m_currentFrame->shadowCasters.push_back(object);

		// Frustrum culling
		if (shouldCull)
		{
			m_currentFrame->frameInfo.culledObjects++;
			continue;
		}

		// Add object to the objects lists
		if (material->GetRenderType() == Resources::RenderType::Opaque)
			m_currentFrame->opaqueObjects.push_back(object);
		else
			m_currentFrame->transpatrentObjects.push_back(object);
	}
}

void DeferRenderFrontend::SetCubemap(Resources::MaterialPtr p_cubemapMat)
{
	if(p_cubemapMat == nullptr)
	{
		m_cubemapObject = nullptr;
	}
	else if(m_previousFrame->cubemapObject == nullptr || reinterpret_cast<size_t>(p_cubemapMat.get()) != m_previousFrame->cubemapObject->id)
	{
		m_currentFrame->renderFlag = m_currentFrame->renderFlag | RendererFlag::RerenderCubeMap;
		RenderObjectPtr object = std::make_shared<RenderObject>();
		object->type = RenderObjectType::CubeMap;
		object->id = reinterpret_cast<size_t>(p_cubemapMat.get());
		object->material = p_cubemapMat;
		m_cubemapObject = object;
	}
	else
		m_cubemapObject = m_previousFrame->cubemapObject;
}

void DeferRenderFrontend::PrepareFrame()
{
	//Swap Frame Buffer Data
	m_previousFrame = m_frameData[m_currentFrameIndex];
	m_currentFrameIndex = ++m_currentFrameIndex % FRAME_DATA_COUNT;
	m_currentFrame = m_frameData[m_currentFrameIndex];

	//Clean new current Frame
	m_currentFrame->opaqueObjects.clear();
	m_currentFrame->shadowCasters.clear();
	m_currentFrame->transpatrentObjects.clear();
	m_currentFrame->debugObjects.clear();
	m_currentFrame->lights.clear();
	m_currentFrame->mainDirectLight = nullptr;
	m_currentFrame->renderFlag = RendererFlag::None;
	m_currentFrame->frameInfo.Reset();

	m_nextDirLightPos = 0;
	m_nextPointLightPos = 0;
	m_nextSpotLightPos = 0;
	m_dirLightShadowNumber = 0;
	m_pointLightShadowNumber = 0;
	m_spotLightShadowNumber = 0;
	m_spotLightNumber = 0;
	m_pointLightNumber = 0;
	m_dirLightNumber = 0;
}

void DeferRenderFrontend::BuildFrame()
{
	m_currentFrame->camera = m_camera;
	m_currentFrame->cubemapObject = m_cubemapObject;

	// Sort objects
	m_currentFrame->opaqueObjects.sort(NormalSort());
	m_currentFrame->debugObjects.sort(NormalSort());
	m_currentFrame->shadowCasters.sort(NormalSort());
	m_currentFrame->transpatrentObjects.sort(TransparentSort());

	m_currentFrame->frameInfo.drawObjects += m_currentFrame->opaqueObjects.size();
	m_currentFrame->frameInfo.drawObjects += m_currentFrame->transpatrentObjects.size();

	//Instanciate objects
	if(m_renderSettings->enableInstancing)
	{
		m_currentFrame->frameInfo.instancedObjects += InstanciateObjectsByMaterial(m_currentFrame->opaqueObjects);
		m_currentFrame->frameInfo.instancedObjects += InstanciateObjectsByMaterial(m_currentFrame->transpatrentObjects);

		// Do not count instanced shadow and debug objects
		InstanciateObjectsByMaterial(m_currentFrame->shadowCasters);
		InstanciateObjectsByMaterial(m_currentFrame->debugObjects);
	}

	//Send objects to the backend renderer
	m_rendererBackend->SetFrame(m_currentFrame);

	m_currentFrame->frameInfo.frameTimeStamp = Utils::Clock::GetInstance().GetRealTime();
	m_currentFrame->frameInfo.frameID = m_frameID++;
}

void DeferRenderFrontend::CalculateFrustrum()
{
	m_camera->RecalculateMatrices();
	m_frustrum.Calculate(m_camera->GetCameraMatrix());
}

void DeferRenderFrontend::DrawDebugCube(const Math::mat4& p_transformMat, bool p_wireframe)
{
	auto renderObj = std::make_shared<RenderObject>();
	renderObj->type = RenderObjectType::Mesh;
	renderObj->material = m_debugMaterial;
	renderObj->worldMat = p_transformMat;
	renderObj->id = 0;
	renderObj->vertexArrayPtr = Resources::Mesh::CreatePrimitive(Resources::Cube)->GetVertexArray();
	renderObj->wiredframe = p_wireframe;

	m_currentFrame->debugObjects.push_back(renderObj);
}

void DeferRenderFrontend::DrawDebugCube(const Math::vec3& p_center, const Math::vec3& p_size, bool p_wireframe)
{
	Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), p_center)
		* Math::scale(Math::mat4(1.0f), p_size);

	DrawDebugCube(transformMat, p_wireframe);
}

void DeferRenderFrontend::DrawDebugSphere(const Math::vec3& p_center, float p_radius, bool p_wireframe)
{
	Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), p_center)
		* Math::scale(Math::mat4(1.0f), PrCore::Math::vec3(p_radius));

	auto renderObj = std::make_shared<RenderObject>();
	renderObj->type = RenderObjectType::Mesh;
	renderObj->material = m_debugMaterial;
	renderObj->worldMat = transformMat;
	renderObj->id = 0;
	renderObj->vertexArrayPtr = Resources::Mesh::CreatePrimitive(Resources::Sphere)->GetVertexArray();
	renderObj->wiredframe = p_wireframe;

	m_currentFrame->debugObjects.push_back(renderObj);
}

void DeferRenderFrontend::DrawDebugLine(const Math::vec3& p_start, const Math::vec3& p_end)
{
	Math::mat4 transformMat = Math::translate(Math::mat4(1.0f), p_start)
		* Math::scale(Math::mat4(1.0f), PrCore::Math::vec3(p_end));

	auto renderObj = std::make_shared<RenderObject>();
	renderObj->type = RenderObjectType::Mesh;
	renderObj->material = m_debugMaterial;
	renderObj->worldMat = transformMat;
	renderObj->id = 0;
	renderObj->wiredframe = true;
	renderObj->vertexArrayPtr = Resources::Mesh::CreatePrimitive(Resources::Line)->GetVertexArray();

	m_currentFrame->debugObjects.push_back(renderObj);
}

void DeferRenderFrontend::SetDebugColor(const Color& p_color)
{
	m_debugColor = p_color;
	m_debugMaterial->SetProperty("color", static_cast<Math::vec4>(p_color));
}

size_t DeferRenderFrontend::InstanciateObjectsByMaterial(RenderObjectVector& p_renderObjects)
{
	RenderObjectVector instanciateCandidates;
	size_t instancedObjects = 0;

	for(auto objIt = p_renderObjects.begin(); objIt != p_renderObjects.end();)
	{
		//First element data
 		const auto object = *objIt;
		const auto materiaHash = object->sortingHash.GetMaterialHash();
		auto vertexArrayID = object->vertexArrayPtr->GetID();

		//Find instance candidates
		size_t instancedCount = 0;
		auto innerIt = objIt;
		while(objIt != p_renderObjects.end() && (*objIt)->sortingHash.GetMaterialHash() == materiaHash && (*objIt)->vertexArrayPtr->GetID() == vertexArrayID)
		{
			instanciateCandidates.push_back(*objIt);
			instancedCount++;
			++objIt;
		}

		//Check if worth instancing
		if (instancedCount > MIN_INSTANCE_COUNT)
		{
			//Grab common data for all instances
			const auto& instnaceFront = instanciateCandidates.front();

			std::vector<PrCore::Math::mat4> matrices;
			matrices.reserve(MAX_INSTANCE_COUNT);
			for (int i = 0; i < instancedCount; )
			{
				const auto innerItBegin = innerIt;
				while (i < instanciateCandidates.size() && matrices.size() < MAX_INSTANCE_COUNT)
				{
					matrices.push_back(std::move((*innerIt)->worldMat));
					i++;
					++innerIt;
				}

				//Create material for instanced group
				const auto instancedVA = instnaceFront->vertexArrayPtr;
				const auto instancedShadowVA = instnaceFront->vertexArrayShadowPtr;
				const auto instancedMat = instnaceFront->material;

				RenderObjectPtr instncedObj = std::make_shared<RenderObject>();
				instncedObj->material = instancedMat;
				instncedObj->vertexArrayPtr = instancedVA;
				instncedObj->vertexArrayShadowPtr = instancedShadowVA;
				instncedObj->id = 0;
				instncedObj->sortingHash = instnaceFront->sortingHash;
				instncedObj->type = RenderObjectType::InstancedMesh;
				instncedObj->instanceSize = matrices.size();
				instncedObj->worldMatrices = matrices;
				instncedObj->wiredframe = object->wiredframe;

				//Erase instanced objects from the buffer
				innerIt = p_renderObjects.erase(innerItBegin, innerIt);
				//Insert instanced object
				objIt = p_renderObjects.insert(innerIt, instncedObj);
				++objIt;

				matrices.clear();
			}

			instancedObjects += instancedCount;
		}

		instanciateCandidates.clear();
	}

	return instancedObjects;
}

size_t DeferRenderFrontend::InstanciateObjectsByMesh(RenderObjectVector& p_renderObjects)
{
	RenderObjectVector instanciateCandidates;
	size_t instancedObjects = 0;

	for (auto objIt = p_renderObjects.begin(); objIt != p_renderObjects.end();)
	{
		//First element data
		const auto object = *objIt;
		auto vertexArrayID = object->vertexArrayPtr->GetID();

		//Find instance candidates
		size_t instancedCount = 0;
		auto innerIt = objIt;
		while (objIt != p_renderObjects.end() && (*objIt)->vertexArrayPtr->GetID() == vertexArrayID)
		{
			instanciateCandidates.push_back(*objIt);
			instancedCount++;
			++objIt;
		}

		//Check if worth instancing
		if (instancedCount > MIN_INSTANCE_COUNT)
		{
			//Grab common data for all instances
			const auto& instnaceFront = instanciateCandidates.front();

			std::vector<PrCore::Math::mat4> matrices;
			matrices.reserve(MAX_INSTANCE_COUNT);
			for (int i = 0; i < instancedCount; )
			{
				const auto innerItBegin = innerIt;
				while (i < instanciateCandidates.size() && matrices.size() < MAX_INSTANCE_COUNT)
				{
					matrices.push_back(std::move((*innerIt)->worldMat));
					i++;
					++innerIt;
				}

				//Create material for instanced group
				const auto instancedVA = instnaceFront->vertexArrayPtr;
				const auto instancedShadowVA = instnaceFront->vertexArrayShadowPtr;

				RenderObjectPtr instncedObj = std::make_shared<RenderObject>();
				instncedObj->vertexArrayPtr = instancedVA;
				instncedObj->vertexArrayShadowPtr = instancedShadowVA;
				instncedObj->id = 0;
				instncedObj->type = RenderObjectType::InstancedMesh;
				instncedObj->instanceSize = matrices.size();
				instncedObj->worldMatrices = matrices;

				//Erase instanced objects from the buffer
				innerIt = p_renderObjects.erase(innerItBegin, innerIt);
				//Insert instanced object
				objIt = p_renderObjects.insert(innerIt, instncedObj);
				++objIt;

				matrices.clear();
			}

			instancedObjects += instancedCount;
		}

		instanciateCandidates.clear();
	}

	return instancedObjects;
}