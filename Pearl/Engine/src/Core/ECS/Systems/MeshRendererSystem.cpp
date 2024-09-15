#include"Core/Common/pearl_pch.h"

#include "Core/ECS/Systems/MeshRendererSystem.h"
#include"Core/Resources/ResourceSystem.h"

#include"Renderer/Core/RenderSystem.h"

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
	renderSystem->SetFlag(RendererFlag::CameraPerspectiveRecalculate);
}

void MeshRendererSystem::OnDisable()
{
}

void MeshRendererSystem::OnUpdate(float p_dt)
{
	renderSystem->CalculateFrustrum();

	for (auto [entity, light, transform] : m_entityViewer.EntitesWithComponents<LightComponent, TransformComponent>())
	{
		renderSystem->SubmitLight(light, transform, entity.GetID().GetID());
	}

	for (auto [entity, mesh, transform] : m_entityViewer.EntitesWithComponents<MeshRendererComponent, TransformComponent>())
	{
		renderSystem->SubmitMesh(entity);
	}
}