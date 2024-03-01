#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Resources/ResourceLoader.h"

#include"Renderer/Core/DeferredRendererFrontend.h"

using namespace PrCore::ECS;
using namespace PrRenderer::Core;

MeshRendererSystem::~MeshRendererSystem()
{
}

void MeshRendererSystem::OnCreate()
{
	m_updateGroup = (uint8_t)UpdateGroup::Custom;
}

void MeshRendererSystem::OnEnable()
{
	DefferedRendererFrontend::GetInstance().SetFlag(RendererFlag::CameraPerspectiveRecalculate);
}

void MeshRendererSystem::OnDisable()
{
}

void MeshRendererSystem::OnUpdate(float p_dt)
{
	for (auto entity : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		auto transform = entity.GetComponent<TransformComponent>();
		auto light = entity.GetComponent<LightComponent>();

		DefferedRendererFrontend::GetInstance().AddLight(light, transform, entity.GetID().GetID());
	}

	for (auto entity : m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		DefferedRendererFrontend::GetInstance().AddMesh(entity);
	}
}