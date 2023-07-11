#include "Core/Common/pearl_pch.h"

#include"Core/ECS/ECS.h"

#include "Renderer/Core/DeferredRendererFrontend.h"
#include "Renderer/Core/BoundingVolume.h"

using namespace PrRenderer::Core;

#define FRAME_DATA_COUNT 2
#define MAX_INSTANCE_COUNT 200
#define MIN_INSTANCE_COUNT 5

DefferedRendererFrontend::DefferedRendererFrontend(IRendererBackend* p_rendererBackend) :
	IRendererFrontend(p_rendererBackend)
{
	//Prepare shaders
	m_instancingShader = PrCore::Resources::ResourceLoader::GetInstance().LoadResource<Resources::Shader>("PBR/PBRwithIR_Instanced.shader");
	PR_ASSERT(m_instancingShader != nullptr, "Instance shader was not found");

	//Prepare frame data
	m_currentFrame = m_frameData[0];
	m_previousFrame = m_frameData[1];
	m_currentFrameIndex = 0;
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

	//Frustrum culling
	const auto frustrum = Frustrum(camera->GetProjectionMatrix(), camera->GetViewMatrix());
	const auto bundingBox = BoxVolume(mesh->GetVertices());
	if (!bundingBox.IsOnFrustrum(frustrum, worldMatrix))
		return;

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
	if (reinterpret_cast<size_t>(p_cubemapMat.get()) != m_previousFrame->cubemapObject->id)
	{
		m_currentFrame->renderFlag = m_currentFrame->renderFlag | RendererFlag::RerenderCubeMap;
		RenderObjectPtr object = std::make_shared<RenderObject>();
		object->type = RenderObjectType::CubeMap;
		object->id = reinterpret_cast<size_t>(p_cubemapMat.get());
		object->material = p_cubemapMat;
		object->mesh = Resources::Mesh::CreatePrimitive(Resources::Quad);
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

}

void DefferedRendererFrontend::BuildFrame()
{
	//Instanciate opaque
	m_currentFrame->opaqueObjects.sort();
	InstanciateObjects(m_currentFrame->opaqueObjects);

	m_currentFrame->transpatrentObjects.sort();
	InstanciateObjects(m_currentFrame->transpatrentObjects);

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
			auto instancedMesh = instnaceFront->mesh;
			auto instancedMat = std::make_shared<Resources::Material>(m_instancingShader);
			instancedMat->CopyPropertiesFrom(*instnaceFront->material);

			std::vector<PrCore::Math::mat4> matrices;
			matrices.reserve(MAX_INSTANCE_COUNT);
			for (int i = 0; i < instancedCount; )
			{
				const auto innerItBegin = innerIt;
				while (i < instanciateCandidates.size() && matrices.size() < MAX_INSTANCE_COUNT)
				{
					matrices.push_back(std::move(instnaceFront->worldMat));
					i++;
					++innerIt;
				}

				instancedMat->SetPropertyArray("modelMatrixArray[0]", matrices.data(), matrices.size());
				RenderObjectPtr instncedObj = std::make_shared<RenderObject>();
				instncedObj->material = instancedMat;
				instncedObj->mesh = instancedMesh;
				instncedObj->id = 0;
				instncedObj->sortingHash = instnaceFront->sortingHash;
				instncedObj->type = RenderObjectType::InstancedMesh;

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
