#include "Editor/Assets/Model/ModelResource.h"

#include "Core/ECS/Scene.h"
#include "Core/ECS/Components/CoreComponents.h"
#include "Core/ECS/Components/RendererComponents.h"
#include "Core/ECS/Components/TransformComponent.h"

using namespace PrEditor::Assets;

ModelResource::ModelResource(std::unique_ptr<ModelEntityGraph>&& p_entityGraph, std::vector<PrRenderer::Resources::MaterialHandle>&& p_materials,
	std::vector<PrRenderer::Resources::MeshHandle>&& p_meshes):
	m_entityGraph(std::move(p_entityGraph)),
	m_materials(std::move(p_materials)),
	m_meshes(std::move(p_meshes))
{
}

void ModelResource::AddEntitesToScene(PrCore::ECS::Scene* p_scene)
{
	PR_ASSERT(p_scene != nullptr, "Scene pointer is null");

	std::unordered_map<std::string_view, PrCore::ECS::Entity> entityMap;
	if (m_entityGraph)
	{
		m_entityGraph->ForEachNodes([&p_scene, &entityMap](const ModelEntityNode* p_node) {

			using namespace PrCore::ECS;

			auto entity = p_node->entity;
			auto parent = p_node->parent;
			auto sceneEntity = p_scene->CreateEntity(entity->name);
			entityMap.insert({ p_node->nodePath , sceneEntity });

			auto transform = sceneEntity.AddComponent<TransformComponent>();
			transform->SetLocalRotation(entity->rotation);
			transform->SetLocalPosition(entity->position);
			transform->SetLocalScale(entity->scale);

			if (parent)
			{
				auto it = entityMap.find(parent->nodePath);
				if (it != entityMap.end())
					sceneEntity.AddComponent<ParentComponent>()->parent = it->second;
				else
					PRLOG_ERROR("Cannot find parent");
			}

			if (!entity->materials.empty() && entity->mesh != nullptr)
			{
				auto meshRenderer = sceneEntity.AddComponent<MeshRendererComponent>();
				meshRenderer->material = entity->materials[0];
				meshRenderer->mesh = entity->mesh;
			}

			if (entity->light)
			{
				auto light = sceneEntity.AddComponent<LightComponent>();
				light->m_light = entity->light;
			}

			// More components in the future
		});
	}
}

size_t ModelResource::GetByteSize() const
{
	size_t size = 0;
	m_entityGraph->ForEachNodes([&size](const ModelEntityNode* p_node) {
		size += sizeof(ModelEntityNode);
		});

	size += m_materials.size() * sizeof(PrRenderer::Resources::MaterialHandle);
	size += m_meshes.size() * sizeof(PrRenderer::Resources::MeshHandle);

	return size;
}

