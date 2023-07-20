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

	//Prepare Lights
	m_maxPShadowLights = (m_renderSettings.comboShadowMap * m_renderSettings.comboShadowMap) / 
		(m_renderSettings.pointLightShadowMapSize * m_renderSettings.pointLightShadowMapSize * 6);
	m_maxSDShadowLights = (m_renderSettings.comboShadowMap * m_renderSettings.comboShadowMap) / 
		(m_renderSettings.lightShadowMapSize * m_renderSettings.lightShadowMapSize);
	m_nextPointLightID = 0;
	m_nextOtherLightsID = 0;

	m_rendererBackend = std::make_shared<DefRendererBackend>(p_settings);

	//Temporary
	m_instancingShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("Deferred/gBuffer.shader");
	PR_ASSERT(m_instancingShader != nullptr, "Instance shader was not found");
}

void DefferedRendererFrontend::AddLight(ECS::LightComponent* p_lightComponent, ECS::TransformComponent* p_transformComponent, size_t p_id)
{
	//Main light
	if(p_lightComponent->mainDirectLight)
	{
		if (m_currentFrame->mainDirectLight)
			PR_ASSERT(false, "FrontendRenderer: Main Direct light already added, cannot have more than one main lights in the scene");

		//If this is main light add it to the frameData and return
		auto lightobject = std::make_shared<LightObject>();
		lightobject->id = p_id;
		lightobject->lightMat = p_lightComponent->m_light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
		lightobject->shadowMapPos = SIZE_MAX;

		m_currentFrame->mainDirectLight = lightobject;
		return;
	}

	//It this is normal light
	if (m_pointLightNumber >m_maxPShadowLights)
	{
		PRLOG_WARN("FrontendRenderer: Discarding point light, max limit exceeded");
		return;
	}
	if(m_otherLightNumber > m_maxSDShadowLights)
	{
		PRLOG_WARN("FrontendRenderer: Discarding light, max limit exceeded");
		return;
	}

	//This functio rebuilts light mapping every frame atm, not very efficient but it is alright for now, to be changed in the future
	auto light = p_lightComponent->m_light;
	LightObject object;
	object.lightMat = light->CreatePackedMatrix(p_transformComponent->GetPosition(), p_transformComponent->GetForwardVector());
	object.id = p_id;
	if (p_lightComponent->m_shadowCast)
	{
		if(light->GetType() == Resources::LightType::Point)
		{
			object.shadowMapPos = m_nextPointLightID;
			m_nextPointLightID += 6;
			m_pointLightNumber++;
		}
		else
		{
			object.shadowMapPos = m_nextOtherLightsID;
			m_nextOtherLightsID++;
			m_otherLightNumber++;
		}
	}
	else
		object.shadowMapPos = SIZE_MAX;

	m_currentFrame->lights.push_back(std::move(object));
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

	//Temporary shader replacement
	//auto material = std::make_shared<Resources::Material>(m_instancingShader);
	//material->CopyPropertiesFrom(*oldmaterial);
	//

	//Frustrum culling
	//diabled for now
	const auto frustrum = Frustrum(camera->GetProjectionMatrix(), camera->GetViewMatrix());
	const auto bundingBox = BoxVolume(mesh->GetVertices());
	//if (!bundingBox.IsOnFrustrum(frustrum, worldMatrix))
		//return;

	//Create renderObject
	RenderObjectPtr object = std::make_shared<RenderObject>();
	object->id = p_entity.GetID().GetID();
	object->material = material;
	object->mesh = mesh;
	object->type = RenderObjectType::Mesh;
	object->worldMat = worldMatrix;

	//Calculate sorting hash
	std::uint32_t hashName = std::hash<std::string>{}(object->material->GetName());
	SortingHash hash(*object);
	hash.SetDepth(RenderUtils::CalculateDepthValue(transformComponent->GetPosition(), camera));
	object->sortingHash = hash;

	//Add object to the correct list
	if (material->GetRenderType() == Resources::RenderType::Opaque)
		m_currentFrame->opaqueObjects.push_back(object);
	else
		m_currentFrame->transpatrentObjects.push_back(object);
}

void DefferedRendererFrontend::AddCubemap(Resources::MaterialPtr p_cubemapMat)
{
	if (m_previousFrame->cubemapObject == nullptr || reinterpret_cast<size_t>(p_cubemapMat.get()) != m_previousFrame->cubemapObject->id)
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
	m_currentFrame->transpatrentObjects.clear();
	m_currentFrame->lights.clear();
	m_currentFrame->mainDirectLight = nullptr;
	m_currentFrame->renderFlag = RendererFlag::None;
	m_nextPointLightID = 0;
	m_nextOtherLightsID = 0;
	m_pointLightNumber = 0;
	m_otherLightNumber = 0;

}

void DefferedRendererFrontend::BuildFrame()
{
	//Instanciate opaque
	m_currentFrame->opaqueObjects.sort();
	InstanciateObjects(m_currentFrame->opaqueObjects);

	m_currentFrame->transpatrentObjects.sort();
	InstanciateObjects(m_currentFrame->transpatrentObjects);

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
				auto instancedMat = std::make_shared<Resources::Material>(m_instancingShader);
				instancedMat->CopyPropertiesFrom(*instnaceFront->material);

				instancedMat->SetPropertyArray("modelMatrixArray[0]", matrices.data(), matrices.size());
				instancedMat->SetProperty("instancedCount", (int)matrices.size());
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
