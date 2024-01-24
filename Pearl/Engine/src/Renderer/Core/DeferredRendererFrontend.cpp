#include "Core/Common/pearl_pch.h"

#include"Core/ECS/ECS.h"

#include "Renderer/Core/DeferredRendererFrontend.h"
#include "Renderer/Core/DefRendererBackend.h"
#include "Renderer/Core/BoundingVolume.h"

using namespace PrRenderer::Core;

#define FRAME_DATA_COUNT 2
#define MAX_INSTANCE_COUNT 200
#define MIN_INSTANCE_COUNT 5

DefferedRendererFrontend::DefferedRendererFrontend(const RendererSettings& p_settings) :
	IRendererFrontend(p_settings)
{
	//Prepare frame data
	m_currentFrame = m_frameData[0];
	m_previousFrame = m_frameData[1];
	m_currentFrameIndex = 0;

	m_nextPointLightPos = 0;
	m_nextSpotLightPos = 0;
	m_nextDirLightPos = 0;

	//Manual Settings Set
	m_renderSettings.cascadeShadowBorders[0] = 0.1f;
	m_renderSettings.cascadeShadowBorders[1] = 0.2f;
	m_renderSettings.cascadeShadowBorders[2] = 0.5f;
	m_renderSettings.cascadeShadowBorders[3] = 1.0f;

	m_rendererBackend = std::make_shared<DefRendererBackend>(m_renderSettings);
}

void DefferedRendererFrontend::AddLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id)
{
	//If Main light
	if(p_lightComponent->mainDirectLight)
	{
		if (m_currentFrame->mainDirectLight)
			PR_ASSERT(false, "FrontendRenderer: Main Direct light already added, cannot have more than one main lights in the scene");

		auto lightObject = std::make_shared<DirLightObject>();
		lightObject->id = p_id;
		lightObject->packedMat = p_lightComponent->m_light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
		lightObject->shadowMapPos = SIZE_MAX; // ShadowMapPos is not inmportant in main light

		m_currentFrame->mainDirectLight = lightObject;
		return;
	}

	//If this is normal light
	if (m_pointLightNumber >m_renderSettings.pointLightMaxShadows)
	{
		PRLOG_WARN("FrontendRenderer: Discarding point light, max limit exceeded");
		return;
	}
	if(m_spotLightNumber > m_renderSettings.spotLightMaxShadows)
	{
		PRLOG_WARN("FrontendRenderer: Discarding spot light, max limit exceeded");
		return;
	}
	if (m_dirLightNumber > m_renderSettings.dirLightMaxShadows)
	{
		PRLOG_WARN("FrontendRenderer: Discarding directional light, max limit exceeded");
		return;
	}

	const auto& light = p_lightComponent->m_light;
	LightObjectPtr lightObject = nullptr;
	switch (light->GetType())
	{
	case Resources::LightType::Directional:
	{
		lightObject = std::make_shared<DirLightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
		lightObject->id = p_id;

		if (p_lightComponent->m_shadowCast)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextDirLightPos;
			m_nextDirLightPos++;
			m_dirLightNumber++;
		}
		break;
	}
	case Resources::LightType::Point:
	{
		lightObject = std::make_shared<LightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
		lightObject->id = p_id;

		if (p_lightComponent->m_shadowCast)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextPointLightPos;
			m_nextPointLightPos += 6;
			m_pointLightNumber++;
		}
		break;
	}
	case Resources::LightType::Spot:
	{
		lightObject = std::make_shared<SpotLightObject>();
		lightObject->packedMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
		lightObject->id = p_id;

		if (p_lightComponent->m_shadowCast)
		{
			lightObject->castShadow = true;
			lightObject->shadowMapPos = m_nextSpotLightPos;
			m_nextSpotLightPos++;
			m_spotLightNumber++;
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

void DefferedRendererFrontend::AddCamera(ECS::CameraComponent* p_camera)
{
	m_currentFrame->camera = p_camera->GetCamera();
}

void DefferedRendererFrontend::AddMesh(ECS::Entity& p_entity)
{
	PR_ASSERT(p_entity.HasComponent<ECS::MeshRendererComponent>(), "FrontendRenderer: entity does not have a MeshRendererComponent");
	PR_ASSERT(p_entity.HasComponent<ECS::TransformComponent>(), "FrontendRenderer: entity does not have a TransformComponent");

	//Preapre data
	auto meshComponent = p_entity.GetComponent<ECS::MeshRendererComponent>();
	auto mesh = meshComponent->mesh;
	auto material = meshComponent->material;
	auto transformComponent = p_entity.GetComponent<ECS::TransformComponent>();
	auto worldMatrix = transformComponent->GetWorldMatrix();
	auto camera = m_currentFrame->camera;


	// Create renderObject
	RenderObjectPtr object = std::make_shared<RenderObject>();
	object->id = p_entity.GetID().GetID();
	object->material = material;
	object->mesh = mesh;
	object->type = RenderObjectType::Mesh;
	object->worldMat = worldMatrix;

	// Calculate sorting hash
	SortingHash hash(*object);
	hash.SetDepth(RenderUtils::CalculateDepthValue(transformComponent->GetPosition(), camera));
	object->sortingHash = hash;

	// Always add to shadowcasters
	if(meshComponent->shadowCaster && material->GetRenderType() == Resources::RenderType::Opaque)
		m_currentFrame->shadowCasters.push_back(object);

	//Frustrum culling
	//Discard objects that are not visable in the main camera
	const auto frustrum = Frustrum(camera->GetProjectionMatrix(), camera->GetViewMatrix());
	const auto bundingBox = BoxVolume(mesh->GetVertices());
	if (!bundingBox.IsOnFrustrum(frustrum, worldMatrix))
		return;

	//Add object to the correct list
	if (material->GetRenderType() == Resources::RenderType::Opaque)
		m_currentFrame->opaqueObjects.push_back(object);
	else
		m_currentFrame->transpatrentObjects.push_back(object);
}

void DefferedRendererFrontend::AddCubemap(Resources::MaterialPtr p_cubemapMat)
{
	if(p_cubemapMat == nullptr)
	{
		m_currentFrame->cubemapObject = nullptr;
	}
	else if(m_previousFrame->cubemapObject == nullptr || reinterpret_cast<size_t>(p_cubemapMat.get()) != m_previousFrame->cubemapObject->id)
	{
		m_currentFrame->renderFlag = m_currentFrame->renderFlag | RendererFlag::RerenderCubeMap;
		RenderObjectPtr object = std::make_shared<RenderObject>();
		object->type = RenderObjectType::CubeMap;
		object->id = reinterpret_cast<size_t>(p_cubemapMat.get());
		object->material = p_cubemapMat;
		m_currentFrame->cubemapObject = object;
	}
	else
		m_currentFrame->cubemapObject = m_previousFrame->cubemapObject;
}

void DefferedRendererFrontend::PrepareFrame()
{
	//Swap Frame Buffer Data
	m_previousFrame = m_frameData[m_currentFrameIndex];
	m_currentFrameIndex = ++m_currentFrameIndex % FRAME_DATA_COUNT;
	m_currentFrame = m_frameData[m_currentFrameIndex];

	//Clean new current Frame
	m_currentFrame->opaqueObjects.clear();
	m_currentFrame->shadowCasters.clear();
	m_currentFrame->transpatrentObjects.clear();
	m_currentFrame->lights.clear();
	m_currentFrame->mainDirectLight = nullptr;
	m_currentFrame->renderFlag = RendererFlag::None;
	m_nextDirLightPos = 0;
	m_nextPointLightPos = 0;
	m_nextSpotLightPos = 0;
	m_dirLightNumber = 0;
	m_pointLightNumber = 0;
	m_spotLightNumber = 0;

}

void DefferedRendererFrontend::BuildFrame()
{
	// Sort objects
	m_currentFrame->opaqueObjects.sort(NormalSort());
	m_currentFrame->shadowCasters.sort(NormalSort());
	m_currentFrame->transpatrentObjects.sort(TransparentSort());

	//Instanciate objects
	if(m_renderSettings.enableInstancing)
	{
		InstanciateObjects(m_currentFrame->opaqueObjects);
		InstanciateObjects(m_currentFrame->shadowCasters);
		InstanciateObjects(m_currentFrame->transpatrentObjects);
	}

	//Send objects to the backend renderer
	m_rendererBackend->SetFrame(m_currentFrame);
}

void DefferedRendererFrontend::InstanciateObjects(RenderObjectVector& p_renderObjects)
{
	RenderObjectVector instanciateCandidates;

	for(auto objIt = p_renderObjects.begin(); objIt != p_renderObjects.end();)
	{
		//First element data
 		const auto object = *objIt;
		auto& hash = object->sortingHash;
		const auto materiaHash = hash.GetMaterialHash();
		const auto renderOrder = hash.GetRenderOrder();
		auto meshName = object->mesh->GetName();

		//Find instance candidates
		size_t instancedCount = 0;
		auto innerIt = objIt;
		while(objIt != p_renderObjects.end() && (*objIt)->sortingHash.GetMaterialHash() == materiaHash && (*objIt)->mesh->GetName() == meshName)
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
				auto instancedMesh = instnaceFront->mesh;
				auto instancedMat = instnaceFront->material;
				//auto instancedMat = std::make_shared<Resources::Material>(*instnaceFront->material);

				//instancedMat->SetPropertyArray("modelMatrixArray[0]", matrices.data(), matrices.size());
				//instancedMat->SetProperty("instancedCount", (int)matrices.size());
				RenderObjectPtr instncedObj = std::make_shared<RenderObject>();
				instncedObj->material = instancedMat;
				instncedObj->mesh = instancedMesh;
				instncedObj->id = 0;
				instncedObj->sortingHash = instnaceFront->sortingHash;
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
		}

		instanciateCandidates.clear();
	}
}
